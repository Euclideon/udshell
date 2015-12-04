#pragma once
#ifndef _EP_MENU_H
#define _EP_MENU_H

#include "components/resources/resource.h"

namespace ep
{
SHARED_CLASS(Menu);

class Menu : public Resource
{
  EP_DECLARE_COMPONENT(Menu, Resource, EPKERNEL_PLUGINVERSION, "Tree model for menus and toolbars")

public:
  Variant GetMenuData() const { return menuData; }
  void SetMenuData(Variant _menuData) { menuData = _menuData; }

  void AddItem(String parentPath, Variant::VarMap properties);
  void AddXMLItems(String parentPath, String xmlStr);
  bool RemoveItem(String path);
  bool SetItemProperties(String path, Variant::VarMap properties);

  Event<> Changed;
protected:
  Menu(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams);

  Variant ParseXMLString(String buffer);
  Variant ParseXMLMenu(Variant inMenu);
  Variant *FindMenuItem(String *parentPath);
  Variant CreateMenuItem(Variant::VarMap properties);
  void SetMenuProperties(Variant &menu, Variant::VarMap properties);

  Variant menuData;

  static Array<const PropertyInfo> GetProperties()
  {
    return{
      EP_MAKE_PROPERTY(MenuData, "Heirarchical structure of menus", nullptr, 0),
    };
  }

  static Array<const MethodInfo> GetMethods()
  {
    return{
      EP_MAKE_METHOD(AddXMLItems, "Add menu items from an XML string"),
      EP_MAKE_METHOD(RemoveItem, "Remove menu item"),
      EP_MAKE_METHOD(AddItem, "Add a menu item with the given properties"),
      EP_MAKE_METHOD(SetItemProperties, "Set properties for the given menu item"),
    };
  }

  static Array<const EventInfo> GetEvents()
  {
    return{
      EP_MAKE_EVENT(Changed, "The menu data has changed"),
    };
  }
};

} // End namespace ep

#endif // _EP_MENU_H
