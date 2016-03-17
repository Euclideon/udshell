#pragma once
#ifndef EPWINDOWIMPL_H
#define EPWINDOWIMPL_H

#include "ep/cpp/component/window.h"
#include "ep/cpp/internal/i/iwindow.h"

namespace ep {

class WindowImpl : public BaseImpl<Window, IWindow>
{
public:
  WindowImpl(Component *pInstance, Variant::VarMap initParams)
    : ImplSuper(pInstance)
  {
  }

  UIComponentRef GetTopLevelUI() const override final { return spTopLevelUI; }

  UIComponentRef spTopLevelUI = nullptr;
};

class WindowGlue final : public Window
{
public:
  WindowGlue(const ComponentDesc *_pType, Kernel *_pKernel, SharedString _uid, ComponentRef _spInstance, Variant::VarMap initParams)
    : Window(_pType, _pKernel, _uid, initParams), spInstance(_spInstance)
  {
  }

protected:
  ComponentRef spInstance;
};

} // namespace ep

#endif // EPWINDOWIMPL_H
