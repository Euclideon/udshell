#pragma once
#ifndef UD_WINDOW_H
#define UD_WINDOW_H

#include "component.h"

namespace kernel
{

PROTOTYPE_COMPONENT(Window);

SHARED_CLASS(UIComponent);

class Window : public Component
{
  EP_DECLARE_COMPONENT(Window, Component, EPKERNEL_PLUGINVERSION, "Window desc...")
public:

  void SetTopLevelUI(UIComponentRef spUIComponent);
  UIComponentRef GetTopLevelUI() const { return spTopLevelUI; }

protected:
  Window(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams);
  virtual ~Window();

  epResult CreateInternal(Variant::VarMap initParams);
  void DestroyInternal();

  UIComponentRef spTopLevelUI = nullptr;

  static Array<const PropertyInfo> GetProperties()
  {
    return{
      EP_MAKE_PROPERTY_EXPLICIT("UI", "Top level UI for the window", EP_MAKE_GETTER(GetTopLevelUI), EP_MAKE_SETTER(SetTopLevelUI), nullptr, 0),
    };
  }
};

} // namespace kernel

#endif // UD_WINDOW_H
