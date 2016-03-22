#pragma once
#ifndef _EP_MENU_H
#define _EP_MENU_H

#include "ep/cpp/component/resource/resource.h"
#include "ep/cpp/internal/i/imenu.h"

namespace ep
{
SHARED_CLASS(Menu);

class Menu : public Resource
{
  EP_DECLARE_COMPONENT_WITH_IMPL(Menu, IMenu, Resource, EPKERNEL_PLUGINVERSION, "Tree model for menus and toolbars", 0)

public:
  Variant GetMenuData() const { return pImpl->GetMenuData(); }
  void SetMenuData(Variant menuData) { pImpl->SetMenuData(menuData); }

  void AddItem(String parentPath, Variant::VarMap properties) { pImpl->AddItem(parentPath, properties); }
  void AddXMLItems(String parentPath, String xmlStr) { pImpl->AddXMLItems(parentPath, xmlStr); }
  bool RemoveItem(String path) { return pImpl->RemoveItem(path); }
  bool SetItemProperties(String path, Variant::VarMap properties) { return pImpl->SetItemProperties(path, properties); }

  Variant Save() const { return pImpl->Save(); }

  Event<> Changed;
protected:
  Menu(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
    : Resource(pType, pKernel, uid, initParams)
  {
    pImpl = CreateImpl(initParams);
  }

  Variant ParseXMLString(String buffer) { return pImpl->ParseXMLString(buffer); }
  Variant ParseXMLMenu(Variant inMenu) { return pImpl->ParseXMLMenu(inMenu); }
  Variant *FindMenuItem(String *parentPath) { return pImpl->FindMenuItem(parentPath); }
  Variant CreateMenuItem(Variant::VarMap properties) { return pImpl->CreateMenuItem(properties); }
  void SetMenuProperties(Variant &menu, Variant::VarMap properties) { pImpl->SetMenuProperties(menu, properties); }

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
