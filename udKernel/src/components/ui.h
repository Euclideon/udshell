#pragma once
#ifndef UI_H
#define UI_H

#include "component.h"

#if UDUI_DRIVER == UDDRIVER_QT
namespace qt {
  class QtComponent;
  class QtKernel;
}
#endif

namespace ud
{

PROTOTYPE_COMPONENT(UIComponent);
PROTOTYPE_COMPONENT(Viewport);
PROTOTYPE_COMPONENT(Window);

class UIComponent : public Component
{
public:
  UD_COMPONENT(UIComponent);

protected:
  friend class qt::QtKernel;

  UIComponent(const ComponentDesc *pType, Kernel *pKernel, udSharedString uid, udInitParams initParams);
  virtual ~UIComponent();

  udResult CreateInternal(udString filename);
  void DestroyInternal();

#if UDUI_DRIVER == UDDRIVER_QT
  qt::QtComponent *pInternal = nullptr;
#else
  void *pInternal = nullptr;
#endif
};

class Viewport : public UIComponent
{
public:
  UD_COMPONENT(Viewport);

protected:
  Viewport(const ComponentDesc *pType, Kernel *pKernel, udRCString uid, udInitParams initParams);
  virtual ~Viewport();

  static Component *Create(const ComponentDesc *pType, Kernel *pKernel, udRCString uid, udInitParams initParams);
};

class Window : public Component
{
public:
  UD_COMPONENT(Window);

  void SetUI(UIComponentRef spUIComponent);
  UIComponentRef GetUI() const { return spTopLevelUI; }

protected:
  Window(const ComponentDesc *pType, Kernel *pKernel, udRCString uid, udInitParams initParams)
    : Component(pType, pKernel, uid, initParams) {}
  virtual ~Window() {}

  virtual void Refresh() { UDASSERT(false, "OVERRIDE THIS METHOD"); }

  static Component *Create(const ComponentDesc *pType, Kernel *pKernel, udRCString uid, udInitParams initParams);

  UIComponentRef spTopLevelUI = nullptr;
};

} // namespace ud

#endif // UI_H
