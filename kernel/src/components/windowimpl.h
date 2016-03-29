#pragma once
#ifndef EPWINDOWIMPL_H
#define EPWINDOWIMPL_H

#include "ep/cpp/component/window.h"
#include "ep/cpp/internal/i/iwindow.h"

namespace ep {

inline Array<const PropertyInfo> Window::GetProperties() const
{
  return{
    EP_MAKE_PROPERTY_EXPLICIT("UI", "Top level UI for the window", EP_MAKE_GETTER(GetTopLevelUI), EP_MAKE_SETTER(SetTopLevelUI), nullptr, 0)
  };
}

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

} // namespace ep

#endif // EPWINDOWIMPL_H
