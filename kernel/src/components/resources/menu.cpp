#include "menu.h"
#include "kernel.h"
#include "rapidxml.hpp"
#include "components/resources/text.h"
#include "components/shortcutmanager.h"

namespace ep
{
static CPropertyDesc props[] =
{
  {
    {
      "menudata", // id
      "MenuData", // displayName
      "Heirarchical structure of menus", // description
    },
    &Menu::GetMenuData, // getter
    &Menu::SetMenuData, // setter
  },
};

static CMethodDesc methods[] =
{
  {
    {
      "addxmlitems", // id
      "Add menu items from an XML string", // description
    },
    &Menu::AddXMLItems, // method
  },
  {
    {
      "removeitem", // id
      "Remove menu item", // description
    },
    &Menu::RemoveItem, // method
  },
  // TODO Uncomment functions when we support params of type Slice<const KeyValuePair>
  /*
  {
    {
      "additem", // id
      "Add a menu item with the given properties", // description
    },
    &Menu::AddItem, // method
  },
  {
    {
      "setitemproperties", // id
      "set properties for the given menu item", // description
    },
    &Menu::SetItemProperties, // method
  },
  */
};

static CEventDesc events[] =
{
  {
    {
      "changed", // id
      "Changed", // displayName
      "The menu data has changed", // description
    },
    &Menu::Changed, // event
  },
};

ComponentDesc Menu::descriptor =
{
  &Resource::descriptor, // pSuperDesc

  EPSHELL_APIVERSION, // epVersion
  EPSHELL_PLUGINVERSION, // pluginVersion

  "menu", // id
  "Menu", // displayName
  "Menu resource", // description

  Slice<CPropertyDesc>(props, UDARRAYSIZE(props)),   // properties
  Slice<CMethodDesc>(methods, UDARRAYSIZE(methods)), // methods
  Slice<CEventDesc>(events, UDARRAYSIZE(events)),    // events
};

Menu::Menu(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, InitParams initParams)
  : Resource(pType, pKernel, uid, initParams)
{
  menuData = Array<Variant>();

  const Variant &src = initParams["src"];
  if (src.is(Variant::Type::String))
  {
    Variant menus = ParseXMLString(src.asString());
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

  using namespace rapidxml;
  Variant rootMenu;

  try
  {
    rootMenu = spXMLBuffer->ParseXml();
  }
  catch (parse_error e)
  {
    LogError("Unable to parse menus xml string on line {0} : {1}", Text::GetLineNumberFromByteIndex(buffer, (size_t)(e.where<char>() - buffer.ptr)), e.what());
    return false;
  }

  Array<Variant> outMenus;
  Slice<KeyValuePair> inMenus = rootMenu.asAssocArray()[0].value.asAssocArray();
  for(KeyValuePair &inMenu : inMenus)
    outMenus.pushBack(ParseXMLMenu(inMenu));

  return outMenus;
}

Variant Menu::ParseXMLMenu(KeyValuePair inMenu)
{
  Array<KeyValuePair> menu;
  Array<Variant> children;
  
  if (!inMenu.value.is(Variant::Type::AssocArray))
    return menu;
  Slice<KeyValuePair> kvps = inMenu.value.asAssocArray();

  for (KeyValuePair &kvp : kvps)
  {
    if (!kvp.key.asString().cmp("_attributes"))
    {
      Slice<KeyValuePair> attributes = kvp.value.asAssocArray();
      for(KeyValuePair attr : attributes)
        menu.pushBack(attr);
    }
    else
      children.pushBack(ParseXMLMenu(kvp));
  }
  menu.pushBack(KeyValuePair("children", children));

  return menu;
}

void Menu::AddXMLItems(String parentPath, String xmlStr)
{
  Variant menus = ParseXMLString(xmlStr);
  Slice<Variant> menuArray = menus.asArray();
  for (Variant &menu : menuArray)
    AddItem(parentPath, menu.asAssocArray());
}

bool Menu::SetItemProperties(String path, Slice<const KeyValuePair> properties)
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

void Menu::AddItem(String parentPath, Slice<const KeyValuePair> properties)
{
  Variant *pChildren;

  Variant subTree = CreateMenuItem(properties);
  Variant *pParent = FindMenuItem(&parentPath);
  if (!pParent)
    pChildren = &menuData;
  else
  {
    Variant &parent = *pParent;    
    pChildren = &parent["children"];
  }
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
        Variant &param = child["name"];
        if (!param.asString().cmp(token))
        {
          itemFound = true;
          *parentPath = path;
          parent = &child;
          children = child["children"].asArray();
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
    
    Variant &parent = *pParent;
    pChildren = &parent["children"];
  }

  Array<Variant> children = pChildren->asArray();

  for (size_t i = 0; i < children.length; i++)
  {
    Variant &childName = children[i]["name"];
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

Variant Menu::CreateMenuItem(Slice<const KeyValuePair> properties)
{
  Array<KeyValuePair> mapArray;
  mapArray.pushBack(KeyValuePair("name", ""));
  mapArray.pushBack(KeyValuePair("checkable", false));
  mapArray.pushBack(KeyValuePair("checked", false));
  mapArray.pushBack(KeyValuePair("exclusivegroup", false));
  mapArray.pushBack(KeyValuePair("separator", false));
  mapArray.pushBack(KeyValuePair("shortcut", ""));
  mapArray.pushBack(KeyValuePair("command", ""));
  mapArray.pushBack(KeyValuePair("children", Array<Variant>()));
  
  Variant map(mapArray);
  SetMenuProperties(map, properties);

  return map;
}

void Menu::SetMenuProperties(Variant &menu, Slice<const KeyValuePair> properties)
{
  Array<Variant> menuChildren;
  String command;

  for (const KeyValuePair &item : properties)
  {
    String itemName = item.key.asString();

    if (!itemName.cmp("command"))
      menu["shortcut"] = pKernel->GetShortcutManager()->GetShortcutString(item.value.asString());
    else if (!itemName.cmp("children"))
    {
      Slice<Variant> children = item.value.asArray();
      for (Variant &child : children)
        menuChildren.pushBack(CreateMenuItem(child.asAssocArray()));

      menu["children"] = menuChildren;
    }
    else
    {
      if (item.value.is(Variant::Type::String))
      {
        if (!item.value.asString().cmp("true"))
          menu[itemName] = Variant(true);
        else if (!item.value.asString().cmp("false"))
          menu[itemName] = Variant(false);
        else
          menu[itemName] = item.value;
      }
      else
        menu[itemName] = item.value;
    }
  }
}

} // namespace ep
