#pragma once
#ifndef UD_WINDOW_H
#define UD_WINDOW_H

#include "component.h"

namespace ud
{

PROTOTYPE_COMPONENT(Window);

SHARED_CLASS(UIComponent);

class Window : public Component
{
public:
  UD_COMPONENT(Window);

  void SetTopLevelUI(UIComponentRef spUIComponent);
  UIComponentRef GetTopLevelUI() const { return spTopLevelUI; }

  void *GetInternalData() { return pInternal; }
  const void *GetInternalData() const { return pInternal; }

protected:
  Window(const ComponentDesc *pType, Kernel *pKernel, udSharedString uid, udInitParams initParams);
  virtual ~Window();

  udResult CreateInternal(udInitParams initParams);
  void DestroyInternal();

  UIComponentRef spTopLevelUI = nullptr;

  void *pInternal = nullptr;
};

} // namespace ud

#endif // UD_WINDOW_H
