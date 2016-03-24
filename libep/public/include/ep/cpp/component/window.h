#pragma once
#if !defined(_EP_WINDOW_HPP)
#define _EP_WINDOW_HPP

#include "ep/cpp/component/component.h"
#include "ep/cpp/component/uicomponent.h"
#include "ep/cpp/internal/i/iwindow.h"

namespace ep {

SHARED_CLASS(Window);

class Window : public Component
{
  EP_DECLARE_COMPONENT_WITH_IMPL(Window, IWindow, Component, EPKERNEL_PLUGINVERSION, "UI Window", ComponentInfoFlags::Abstract)
public:
  void SetTopLevelUI(UIComponentRef spUIComponent) { pImpl->SetTopLevelUI(spUIComponent); }
  UIComponentRef GetTopLevelUI() const { return pImpl->GetTopLevelUI(); }

protected:
  Window(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
    : Component(pType, pKernel, uid, initParams)
  {
    pImpl = CreateImpl(initParams);
  }

private:
  Array<const PropertyInfo> GetProperties() const;
};

} // namespace ep

#endif // _EP_WINDOW_HPP
