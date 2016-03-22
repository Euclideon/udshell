#pragma once
#if !defined(_EP_WINDOW_HPP)
#define _EP_WINDOW_HPP

#include "ep/cpp/component/component.h"
#include "ep/cpp/component/uicomponent.h"
#include "ep/cpp/internal/i/iwindow.h"

namespace ep {

SHARED_CLASS(Window);

class Window : public Component, public IWindow
{
  EP_DECLARE_COMPONENT_WITH_IMPL(Window, IWindow, Component, EPKERNEL_PLUGINVERSION, "UI Window", ComponentInfoFlags::Abstract)
public:
  void SetTopLevelUI(UIComponentRef spUIComponent) override final { pImpl->SetTopLevelUI(spUIComponent); }
  UIComponentRef GetTopLevelUI() const override final { return pImpl->GetTopLevelUI(); }

protected:
  Window(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
    : Component(pType, pKernel, uid, initParams)
  {
    pImpl = CreateImpl(initParams);
  }

  static Array<const PropertyInfo> GetProperties()
  {
    return{
      EP_MAKE_PROPERTY_EXPLICIT("UI", "Top level UI for the window", EP_MAKE_GETTER(GetTopLevelUI), EP_MAKE_SETTER(SetTopLevelUI), nullptr, 0)
    };
  }
};

} // namespace ep

#endif // _EP_WINDOW_HPP
