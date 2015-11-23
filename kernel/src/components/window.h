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
public:
  EP_COMPONENT(Window);

  void SetTopLevelUI(UIComponentRef spUIComponent);
  UIComponentRef GetTopLevelUI() const { return spTopLevelUI; }

  void *GetInternalData() { return pInternal; }
  const void *GetInternalData() const { return pInternal; }

protected:
  Window(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, InitParams initParams);
  virtual ~Window();

  epResult CreateInternal(InitParams initParams);
  void DestroyInternal();

  UIComponentRef spTopLevelUI = nullptr;

  void *pInternal = nullptr;
};

} // namespace kernel

#endif // UD_WINDOW_H
