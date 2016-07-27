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
  EP_DECLARE_COMPONENT_WITH_IMPL(ep, Menu, IMenu, Resource, EPKERNEL_PLUGINVERSION, "Tree model for menus and toolbars", 0)

public:
  Variant GetMenuData() const { return pImpl->GetMenuData(); }
  void SetMenuData(Variant menuData) { pImpl->SetMenuData(menuData); }

  void AddItem(String parentPath, Variant::VarMap properties) { pImpl->AddItem(parentPath, properties); }
  void AddXMLItems(String parentPath, String xmlStr) { pImpl->AddXMLItems(parentPath, xmlStr); }
  bool RemoveItem(String path) { return pImpl->RemoveItem(path); }
  bool SetItemProperties(String path, Variant::VarMap properties) { return pImpl->SetItemProperties(path, properties); }

  Variant save() const { return pImpl->Save(); }

  Event<> Changed;
protected:
  Menu(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
    : Resource(pType, pKernel, uid, initParams)
  {
    pImpl = createImpl(initParams);
  }

  Variant ParseXMLString(String buffer) { return pImpl->ParseXMLString(buffer); }
  Variant ParseXMLMenu(Variant inMenu) { return pImpl->ParseXMLMenu(inMenu); }
  Variant *FindMenuItem(String *parentPath) { return pImpl->FindMenuItem(parentPath); }
  Variant CreateMenuItem(Variant::VarMap properties) { return pImpl->CreateMenuItem(properties); }
  void SetMenuProperties(Variant &menu, Variant::VarMap properties) { pImpl->SetMenuProperties(menu, properties); }

private:
  Array<const PropertyInfo> getProperties() const;
  Array<const MethodInfo> getMethods() const;
  Array<const EventInfo> getEvents() const;
};

} // End namespace ep

#endif // _EP_MENU_H
