#pragma once
#ifndef UI_H
#define UI_H

#include "component.h"

namespace kernel
{

PROTOTYPE_COMPONENT(UIComponent);

class UIComponent : public Component
{
  EP_DECLARE_COMPONENT(UIComponent, Component, EPKERNEL_PLUGINVERSION, "UIComponent desc...")
public:

  Variant GetUIHandle() const;

protected:
  UIComponent(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, InitParams initParams);
  virtual ~UIComponent();

  epResult CreateInternal(InitParams initParams);
  epResult InitComplete() override;
  void DestroyInternal();

  static Array<const PropertyInfo> GetProperties()
  {
    return{
      EP_MAKE_PROPERTY_RO(UIHandle, "Platform specific UI handle", nullptr, 0),
    };
  }
};

} // namespace kernel

#endif // UI_H
