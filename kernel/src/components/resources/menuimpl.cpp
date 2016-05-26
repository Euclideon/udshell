#include "menuimpl.h"
#include "ep/cpp/kernel.h"
#include "rapidxml.hpp"
#include "ep/cpp/component/resource/text.h"
#include "ep/cpp/component/commandmanager.h"

namespace ep {

Array<const PropertyInfo> Menu::GetProperties() const
{
  return{
    EP_MAKE_PROPERTY(MenuData, "Heirarchical structure of menus", nullptr, 0),
  };
}
Array<const MethodInfo> Menu::GetMethods() const
{
  return{
    EP_MAKE_METHOD(AddXMLItems, "Add menu items from an XML string"),
    EP_MAKE_METHOD(RemoveItem, "Remove menu item"),
    EP_MAKE_METHOD(AddItem, "Add a menu item with the given properties"),
    EP_MAKE_METHOD(SetItemProperties, "Set properties for the given menu item"),
  };
}
Array<const EventInfo> Menu::GetEvents() const
{
  return{
    EP_MAKE_EVENT(Changed, "The menu data has changed"),
  };
}

MenuImpl::MenuImpl(Component *pInstance, Variant::VarMap initParams)
  : ImplSuper(pInstance)
{
  menuData = Array<Variant>();

  Variant *pSrc = initParams.get("src");
  if (pSrc && pSrc->is(Variant::Type::String))
  {
    Variant menus = ParseXMLString(pSrc->asString());
    Slice<Variant> menuArray = menus.asArray();
    for (Variant &menu : menuArray)
      AddItem("", menu.asAssocArray());
  }
}

Variant MenuImpl::ParseXMLString(String buffer)
{
  auto spXMLBuffer = GetKernel()->CreateComponent<Text>();
  spXMLBuffer->Allocate(buffer.length + 1);
  spXMLBuffer->CopyBuffer(buffer);
  Slice<void> buf = spXMLBuffer->Map();
  buf[buffer.length] = '\0';
  spXMLBuffer->Unmap();

  using namespace rapidxml;
  Variant rootNode;

  try
  {
    rootNode = spXMLBuffer->ParseXml();
  }
  catch (parse_error &e)
  {
    LogError("Unable to parse menus xml string on line {0} : {1}", Text::GetLineNumberFromByteIndex(buffer, (size_t)(e.where<char>() - buffer.ptr)), e.what());
    return false;
  }

  Array<Variant> outMenus;
  Variant *pInMenus = rootNode.getItem("children");
  if (pInMenus && pInMenus->is(Variant::Type::Array))
  {
    Slice<Variant> inMenus = pInMenus->asArray();
    for (auto inMenu : inMenus)
      outMenus.pushBack(ParseXMLMenu(inMenu));
  }

  return outMenus;
}

Variant MenuImpl::ParseXMLMenu(Variant inMenu)
{
  Variant::VarMap menu;
  Array<Variant> menuChildren;

  if (!inMenu.is(Variant::SharedPtrType::AssocArray))
    return menu;
  Variant::VarMap inMap = inMenu.asAssocArray();

  Variant *pName = inMap.get("name");
  if(pName && pName->is(Variant::Type::String))
    menu.insert("type", pName->asString());

  Variant *pAttributes = inMap.get("attributes");
  if (pAttributes && pAttributes->is(Variant::SharedPtrType::AssocArray))
  {
    Variant::VarMap attributes = pAttributes->asAssocArray();
    for (auto attr : attributes)
      menu.insert(attr);
  }

  Variant *pChildren = inMap.get("children");
  if (pChildren && pChildren->is(Variant::Type::Array))
  {
    Slice<Variant> children = pChildren->asArray();
    for (auto child : children)
      menuChildren.pushBack(ParseXMLMenu(child));

    menu.insert("children", menuChildren);
  }

  return menu;
}

void MenuImpl::AddXMLItems(String parentPath, String xmlStr)
{
  Variant menus = ParseXMLString(xmlStr);
  Slice<Variant> menuArray = menus.asArray();
  for (Variant &menu : menuArray)
    AddItem(parentPath, menu.asAssocArray());
}

bool MenuImpl::SetItemProperties(String path, Variant::VarMap properties)
{
  Variant *pParent = FindMenuItem(&path);
  if (!pParent)
  {
    GetKernel()->LogDebug(2, "Can't set properties of non-existent menu item \"{0}\"", path);
    return false;
  }

  SetMenuProperties(*pParent, properties);
  pInstance->Changed.Signal();

  return true;
}

void MenuImpl::AddItem(String parentPath, Variant::VarMap properties)
{
  Variant *pChildren;

  Variant subTree = CreateMenuItem(properties);
  Variant *pParent = FindMenuItem(&parentPath);
  if (!pParent)
    pChildren = &menuData;
  else
    pChildren = pParent->getItem("children");

  Array<Variant> children = pChildren->asArray();

  children.pushBack(subTree);
  *pChildren = Variant(std::move(children));

  pInstance->Changed.Signal();
}

Variant *MenuImpl::FindMenuItem(String *parentPath)
{
  bool itemFound = false;
  Slice<Variant> children = menuData.asArray();
  Variant *parent = nullptr;
  String path = *parentPath;

  while (!path.empty())
  {
    String token = path.popToken("/");
    if (!token.empty())
    {
      for (Variant &child : children)
      {
        Variant &param = *child.getItem("name");
        if (!param.asString().cmp(token))
        {
          itemFound = true;
          *parentPath = path;
          parent = &child;
          children = child.getItem("children")->asArray();
        }
      }
      if (!itemFound)
        return parent;
    }
  }

  return parent;
}

bool MenuImpl::RemoveItem(String path)
{
  Variant *pParent = nullptr;
  Variant *pChildren = nullptr;
  String removeName;

  String parentPath = path.getLeftAtLast(String("/"), false);
  if (parentPath.length == path.length)
  {
    pChildren = &menuData;
    removeName = path;
  }
  else if(!parentPath.empty())
  {
    removeName = path.getRightAtLast(String("/"), false);

    pParent = FindMenuItem(&parentPath);
    if (!pParent || !parentPath.empty())
    {
      LogDebug(2, "Can't remove non-existent menu item \"{0}\"", path);
      return false;
    }

    pChildren = pParent->getItem("children");
  }

  Array<Variant> children = pChildren->asArray();

  for (size_t i = 0; i < children.length; i++)
  {
    Variant &childName = *children[i].getItem("name");
    if (!childName.asString().cmp(removeName))
    {
      children.remove(i);
      break;
    }
  }
  *pChildren = Variant(std::move(children));

  pInstance->Changed.Signal();

  return true;
}

Variant MenuImpl::CreateMenuItem(Variant::VarMap properties)
{
  Variant::VarMap map;
  map.insert(KeyValuePair("type", ""));
  map.insert(KeyValuePair("name", ""));
  map.insert(KeyValuePair("description", ""));
  map.insert(KeyValuePair("image", ""));
  map.insert(KeyValuePair("checkable", false));
  map.insert(KeyValuePair("checked", false));
  map.insert(KeyValuePair("exclusivegroup", false));
  map.insert(KeyValuePair("enabled", true));
  map.insert(KeyValuePair("shortcut", ""));
  map.insert(KeyValuePair("command", ""));
  map.insert(KeyValuePair("split", true));
  map.insert(KeyValuePair("children", Array<Variant>()));

  Variant varMap(map);

  SetMenuProperties(varMap, properties);

  return varMap;
}

void MenuImpl::SetMenuProperties(Variant &menu, Variant::VarMap properties)
{
  Array<Variant> menuChildren;
  String command;

  for (auto item : properties)
  {
    String itemName = item.key.asString();

    if (itemName.eq("command"))
      *menu.getItem("shortcut") = GetKernel()->GetCommandManager()->GetShortcut(item.value.asString());

    if (itemName.eq("children"))
    {
      Slice<Variant> children = item.value.asArray();
      for (Variant &child : children)
        menuChildren.pushBack(CreateMenuItem(child.asAssocArray()));

      *menu.getItem("children") = menuChildren;
    }
    else
    {
      if (item.value.is(Variant::Type::String))
      {
        if (item.value.asString().eq("true"))
          *menu.getItem(itemName) = Variant(true);
        else if (item.value.asString().eq("false"))
          *menu.getItem(itemName) = Variant(false);
        else
          *menu.getItem(itemName) = item.value;
      }
      else
        *menu.getItem(itemName) = item.value;
    }
  }
}

} // namespace ep
