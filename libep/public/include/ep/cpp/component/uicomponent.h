#pragma once
#if !defined(_EP_UICOMPONENT_HPP)
#define _EP_UICOMPONENT_HPP

#include "ep/cpp/component/component.h"
#include "ep/cpp/internal/i/iuicomponent.h"

namespace ep {

SHARED_CLASS(UIComponent);

class UIComponent : public Component
{
  EP_DECLARE_COMPONENT_WITH_IMPL(ep, UIComponent, IUIComponent, Component, EPKERNEL_PLUGINVERSION, "Base UI Component", ComponentInfoFlags::Abstract)
public:
  Variant GetUIHandle() const { return pImpl->GetUIHandle(); }

  virtual void PostInit(void *pData) { pImpl->PostInit(pData); }

protected:
  UIComponent(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
    : Component(pType, pKernel, uid, initParams)
  {
    pImpl = createImpl(initParams);
  }

private:
  Array<const PropertyInfo> getProperties() const;
};

} // namespace ep

#endif // _EP_UICOMPONENT_HPP
