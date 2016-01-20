#pragma once
#if !defined(_EP_VIEWPORT_HPP)
#define _EP_VIEWPORT_HPP

#include "ep/cpp/component/uicomponent.h"
#include "ep/cpp/internal/i/iviewport.h"
#include "ep/cpp/component/view.h"

namespace ep {

SHARED_CLASS(Viewport);

class Viewport : public UIComponent, public IViewport
{
  EP_DECLARE_COMPONENT_WITH_IMPL(Viewport, IViewport, UIComponent, EPKERNEL_PLUGINVERSION, "UI Rendering Viewport")
public:
  ViewRef GetView() const override final { return pImpl->GetView(); }
  void SetView(ViewRef spView) override final { pImpl->SetView(spView); }

protected:
  Viewport(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
    : UIComponent(pType, pKernel, uid, initParams)
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
      EP_MAKE_PROPERTY(View, "The view component used for rendering", nullptr, 0),
    };
  }
};

} // namespace ep

#endif // _EP_VIEWPORT_HPP
