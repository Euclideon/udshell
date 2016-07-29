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
  EP_DECLARE_COMPONENT_WITH_IMPL(ep, Window, IWindow, Component, EPKERNEL_PLUGINVERSION, "UI Window", ComponentInfoFlags::Abstract)
public:
  void setTopLevelUI(UIComponentRef spUIComponent) { pImpl->SetTopLevelUI(spUIComponent); }
  UIComponentRef getTopLevelUI() const { return pImpl->GetTopLevelUI(); }

  void postInit(void *pData) { pImpl->PostInit(pData); }

protected:
  Window(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
    : Component(pType, pKernel, uid, initParams)
  {
    pImpl = createImpl(initParams);
  }

private:
  Array<const PropertyInfo> getProperties() const;
};

} // namespace ep

#endif // _EP_WINDOW_HPP
