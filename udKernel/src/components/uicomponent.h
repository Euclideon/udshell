#pragma once
#ifndef UICOMPONENT_H
#define UICOMPONENT_H

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
};


} // namespace ud

#endif // UICOMPONENT_H
