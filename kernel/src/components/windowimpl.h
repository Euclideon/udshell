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
    CreateInternal(initParams);
  }

  void SetTopLevelUI(UIComponentRef spUIComponent) override final;
  UIComponentRef GetTopLevelUI() const override final { return spTopLevelUI; }

private:
  ~WindowImpl() { DestroyInternal(); }

  void CreateInternal(Variant::VarMap initParams);
  void DestroyInternal();

  UIComponentRef spTopLevelUI = nullptr;
};

} // namespace ep

#endif // EPWINDOWIMPL_H
