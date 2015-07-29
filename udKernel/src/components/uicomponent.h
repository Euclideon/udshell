#pragma once
#ifndef UDUICOMPONENT_H
#define UDUICOMPONENT_H

#include "component.h"

namespace ud
{
PROTOTYPE_COMPONENT(UIComponent);

class UIComponent : public Component
{
public:
  UD_COMPONENT(UIComponent);

protected:
  UIComponent(const ComponentDesc *pType, Kernel *pKernel, udRCString uid, InitParams initParams)
    : Component(pType, pKernel, uid, initParams) {}
  virtual ~UIComponent() {}

  static Component *Create(const ComponentDesc *pType, Kernel *pKernel, udRCString uid, InitParams initParams);
};


} // namespace ud

#endif // UDUICOMPONENT_H
