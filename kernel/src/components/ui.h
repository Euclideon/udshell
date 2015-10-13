#pragma once
#ifndef UI_H
#define UI_H

#include "component.h"

namespace ep
{

PROTOTYPE_COMPONENT(UIComponent);

class UIComponent : public Component
{
public:
  EP_COMPONENT(UIComponent);

  void *GetInternalData() { return pInternal; }
  const void *GetInternalData() const { return pInternal; }

protected:
  UIComponent(const ComponentDesc *pType, Kernel *pKernel, epSharedString uid, epInitParams initParams);
  virtual ~UIComponent();

  epResult CreateInternal(epInitParams initParams);
  epResult InitComplete() override;
  void DestroyInternal();

  void *pInternal = nullptr;
};

} // namespace ep

#endif // UI_H
