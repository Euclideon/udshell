#pragma once
#ifndef _EP_MENU_H
#define _EP_MENU_H

#include "components/resources/resource.h"

namespace ep
{
PROTOTYPE_COMPONENT(Menu);

class Menu : public Resource
{
public:
  EP_COMPONENT(Menu);

  Variant GetMenuData() const { return menuData; }
  void SetMenuData(Variant menuData) { this->menuData = menuData; }

  void AddItem(String parentPath, Slice<const KeyValuePair> properties);
  void AddXMLItems(String parentPath, String xmlStr);
  bool RemoveItem(String path);
  bool SetItemProperties(String path, Slice<const KeyValuePair> properties);

  Event<> Changed;
protected:
  Menu(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, InitParams initParams);

  Variant ParseXMLString(String buffer);
  Variant ParseXMLMenu(KeyValuePair inMenu);
  Variant *FindMenuItem(String *parentPath);
  Variant CreateMenuItem(Slice<const KeyValuePair> properties);
  void SetMenuProperties(Variant &menu, Slice<const KeyValuePair> properties);

  Variant menuData;
};

}

#endif // _EP_MENU_H
