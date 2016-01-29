#pragma once
#ifndef _EP_MENUIMPL_H
#define _EP_MENUIMPL_H

#include "ep/cpp/component/resource/menu.h"
#include "ep/cpp/internal/i/imenu.h"
#include "ep/cpp/component/resource/resource.h"

namespace ep
{

class MenuImpl : public BaseImpl<Menu, IMenu>
{
public:
  MenuImpl(Component *pInstance, Variant::VarMap initParams);

  Variant GetMenuData() const override final { return menuData; }
  void SetMenuData(Variant _menuData) override final { menuData = _menuData; }

  void AddItem(String parentPath, Variant::VarMap properties) override final;
  void AddXMLItems(String parentPath, String xmlStr) override final;
  bool RemoveItem(String path) override final;
  bool SetItemProperties(String path, Variant::VarMap properties) override final;

  Variant Save() const override final { return pInstance->Super::Save(); }

protected:
  Variant ParseXMLString(String buffer) override final;
  Variant ParseXMLMenu(Variant inMenu) override final;
  Variant *FindMenuItem(String *parentPath) override final;
  Variant CreateMenuItem(Variant::VarMap properties) override final;
  void SetMenuProperties(Variant &menu, Variant::VarMap properties) override final;

  Variant menuData;
};

} // End namespace ep

#endif // _EP_MENUIMPL_H
