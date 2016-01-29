#pragma once
#if !defined(_EP_IMENU_HPP)
#define _EP_IMENU_HPP

namespace ep {

class IMenu
{
public:
  virtual Variant GetMenuData() const = 0;
  virtual void SetMenuData(Variant menuData) = 0;

  virtual void AddItem(String parentPath, Variant::VarMap properties) = 0;
  virtual void AddXMLItems(String parentPath, String xmlStr) = 0;
  virtual bool RemoveItem(String path) = 0;
  virtual bool SetItemProperties(String path, Variant::VarMap properties) = 0;

  virtual Variant Save() const = 0;

protected:
  virtual Variant ParseXMLString(String buffer) = 0;
  virtual Variant ParseXMLMenu(Variant inMenu) = 0;
  virtual Variant *FindMenuItem(String *parentPath) = 0;
  virtual Variant CreateMenuItem(Variant::VarMap properties) = 0;
  virtual void SetMenuProperties(Variant &menu, Variant::VarMap properties) = 0;
};

} // End namespace ep

#endif // _EP_IMENU_H
