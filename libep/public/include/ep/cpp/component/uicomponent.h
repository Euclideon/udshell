#pragma once
#if !defined(_EP_UICOMPONENT_HPP)
#define _EP_UICOMPONENT_HPP

#include "ep/cpp/component/component.h"
#include "ep/cpp/internal/i/iuicomponent.h"

namespace ep {

SHARED_CLASS(UIComponent);

class UIComponent : public Component, public IUIComponent
{
  EP_DECLARE_COMPONENT_WITH_IMPL(UIComponent, IUIComponent, Component, EPKERNEL_PLUGINVERSION, "Base UI Component")
public:
  Variant GetUIHandle() const override final { return pImpl->GetUIHandle(); }

protected:
  UIComponent(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
    : Component(pType, pKernel, uid, initParams)
  {
    pImpl = CreateImpl(initParams);
  }

  void InitComplete() override
  {
    Super::InitComplete();
    pImpl->InitComplete();
  }

  static Array<const PropertyInfo> GetProperties()
  {
    return{
      EP_MAKE_PROPERTY_RO(UIHandle, "Platform specific UI handle", nullptr, 0),
    };
  }
};

} // namespace ep

#endif // _EP_UICOMPONENT_HPP
