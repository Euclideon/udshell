#pragma once
#ifndef UI_H
#define UI_H

#include "component.h"

namespace ud
{

PROTOTYPE_COMPONENT(UIComponent);

class UIComponent : public Component
{
public:
  UD_COMPONENT(UIComponent);

  void *GetInternalData() { return pInternal; }
  const void *GetInternalData() const { return pInternal; }

protected:
  UIComponent(const ComponentDesc *pType, Kernel *pKernel, udSharedString uid, udInitParams initParams);
  virtual ~UIComponent();

  udResult CreateInternal(udInitParams initParams);
  void DestroyInternal();

  void *pInternal = nullptr;
};

} // namespace ud

#endif // UI_H
