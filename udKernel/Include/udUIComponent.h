#pragma once
#ifndef UDUICOMPONENT_H
#define UDUICOMPONENT_H

#include "udComponent.h"

namespace udKernel
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


} // namespace udKernel

#endif // UDUICOMPONENT_H
