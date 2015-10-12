#pragma once
#ifndef UD_WINDOW_H
#define UD_WINDOW_H

#include "component.h"

namespace ep
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
  Window(const ComponentDesc *pType, Kernel *pKernel, epSharedString uid, epInitParams initParams);
  virtual ~Window();

  epResult CreateInternal(epInitParams initParams);
  void DestroyInternal();

  UIComponentRef spTopLevelUI = nullptr;

  void *pInternal = nullptr;
};

} // namespace ep

#endif // UD_WINDOW_H
