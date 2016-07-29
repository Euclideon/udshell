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
  Variant getMenuData() const { return pImpl->GetMenuData(); }
  void setMenuData(Variant menuData) { pImpl->SetMenuData(menuData); }

  void addItem(String parentPath, Variant::VarMap properties) { pImpl->AddItem(parentPath, properties); }
  void addXMLItems(String parentPath, String xmlStr) { pImpl->AddXMLItems(parentPath, xmlStr); }
  bool removeItem(String path) { return pImpl->RemoveItem(path); }
  bool setItemProperties(String path, Variant::VarMap properties) { return pImpl->SetItemProperties(path, properties); }

  Variant save() const { return pImpl->Save(); }

protected:
  Menu(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
    : Resource(pType, pKernel, uid, initParams)
  {
    pImpl = createImpl(initParams);
  }

  Variant parseXMLString(String buffer) { return pImpl->ParseXMLString(buffer); }
  Variant parseXMLMenu(Variant inMenu) { return pImpl->ParseXMLMenu(inMenu); }
  Variant *findMenuItem(String *parentPath) { return pImpl->FindMenuItem(parentPath); }
  Variant createMenuItem(Variant::VarMap properties) { return pImpl->CreateMenuItem(properties); }
  void setMenuProperties(Variant &menu, Variant::VarMap properties) { pImpl->SetMenuProperties(menu, properties); }

private:
  Array<const PropertyInfo> getProperties() const;
  Array<const MethodInfo> getMethods() const;
};

} // End namespace ep

#endif // _EP_MENU_H
