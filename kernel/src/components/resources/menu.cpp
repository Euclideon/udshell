#include "menu.h"
#include "kernel.h"
#include "rapidxml.hpp"
#include "components/resources/text.h"
#include "ep/cpp/component/commandmanager.h"

namespace ep
{

Menu::Menu(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
  : Resource(pType, pKernel, uid, initParams)
{
  menuData = Array<Variant>();

  Variant *pSrc = initParams.Get("src");
  if (pSrc && pSrc->is(Variant::Type::String))
  {
    Variant menus = ParseXMLString(pSrc->asString());
    Slice<Variant> menuArray = menus.asArray();
    for (Variant &menu : menuArray)
      AddItem("", menu.asAssocArray());
  }
}

Variant Menu::ParseXMLString(String buffer)
{
  auto spXMLBuffer = pKernel->CreateComponent<Text>();
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
  Variant *pInMenus = rootNode.asAssocArray().Get("children");
  if (pInMenus && pInMenus->is(Variant::Type::Array))
  {
    Slice<Variant> inMenus = pInMenus->asArray();
    for (auto inMenu : inMenus)
      outMenus.pushBack(ParseXMLMenu(inMenu));
  }

  return outMenus;
}

Variant Menu::ParseXMLMenu(Variant inMenu)
{
  Variant::VarMap menu;
  Array<Variant> menuChildren;

  if (!inMenu.is(Variant::Type::AssocArray))
    return menu;
  Variant::VarMap inMap = inMenu.asAssocArray();

  Variant *pName = inMap.Get("name");
  if(pName && pName->is(Variant::Type::String))
    menu.Insert("type", pName->asString());

  Variant *pAttributes = inMap.Get("attributes");
  if (pAttributes && pAttributes->is(Variant::Type::AssocArray))
  {
    Variant::VarMap attributes = pAttributes->asAssocArray();
    for (auto attr : attributes)
      menu.Insert(attr);
  }

  Variant *pChildren = inMap.Get("children");
  if (pChildren && pChildren->is(Variant::Type::Array))
  {
    Slice<Variant> children = pChildren->asArray();
    for (auto child : children)
      menuChildren.pushBack(ParseXMLMenu(child));

    menu.Insert("children", menuChildren);
  }

  return menu;
}

void Menu::AddXMLItems(String parentPath, String xmlStr)
{
  Variant menus = ParseXMLString(xmlStr);
  Slice<Variant> menuArray = menus.asArray();
  for (Variant &menu : menuArray)
    AddItem(parentPath, menu.asAssocArray());
}

bool Menu::SetItemProperties(String path, Variant::VarMap properties)
{
  Variant *pParent = FindMenuItem(&path);
  if (!pParent)
  {
    pKernel->LogDebug(2, "Can't set properties of non-existent menu item \"{0}\"", path);
    return false;
  }

  SetMenuProperties(*pParent, properties);
  Changed.Signal();

  return true;
}

void Menu::AddItem(String parentPath, Variant::VarMap properties)
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

  Changed.Signal();
}

Variant *Menu::FindMenuItem(String *parentPath)
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

bool Menu::RemoveItem(String path)
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

  Changed.Signal();

  return true;
}

Variant Menu::CreateMenuItem(Variant::VarMap properties)
{
  Variant::VarMap map;
  map.Insert(KeyValuePair("type", ""));
  map.Insert(KeyValuePair("name", ""));
  map.Insert(KeyValuePair("description", ""));
  map.Insert(KeyValuePair("image", ""));
  map.Insert(KeyValuePair("checkable", false));
  map.Insert(KeyValuePair("checked", false));
  map.Insert(KeyValuePair("exclusivegroup", false));
  map.Insert(KeyValuePair("enabled", true));
  map.Insert(KeyValuePair("separator", false));
  map.Insert(KeyValuePair("shortcut", ""));
  map.Insert(KeyValuePair("command", ""));
  map.Insert(KeyValuePair("children", Array<Variant>()));

  Variant varMap(map);

  SetMenuProperties(varMap, properties);

  return varMap;
}

void Menu::SetMenuProperties(Variant &menu, Variant::VarMap properties)
{
  Array<Variant> menuChildren;
  String command;

  for (auto item : properties)
  {
    String itemName = item.key.asString();

    if (itemName.eq("command"))
      *menu.getItem("shortcut") = GetKernel().GetCommandManager()->GetShortcut(item.value.asString());

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
