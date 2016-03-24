#pragma once
#if !defined(_EP_VIEWPORT_HPP)
#define _EP_VIEWPORT_HPP

#include "ep/cpp/component/uicomponent.h"
#include "ep/cpp/internal/i/iviewport.h"
#include "ep/cpp/component/view.h"

namespace ep {

SHARED_CLASS(Viewport);

class Viewport : public UIComponent
{
  EP_DECLARE_COMPONENT_WITH_IMPL(Viewport, IViewport, UIComponent, EPKERNEL_PLUGINVERSION, "UI Rendering Viewport", ComponentInfoFlags::Abstract)
public:
  ViewRef GetView() const { return pImpl->GetView(); }
  void SetView(const ViewRef &spView) { pImpl->SetView(spView); }

  void PostInit(void *pData) override { pImpl->PostInit(pData); }

protected:
  Viewport(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
    : UIComponent(pType, pKernel, uid, initParams)
  {
    pImpl = CreateImpl(initParams);
  }

private:
  Array<const PropertyInfo> GetProperties() const;
};

} // namespace ep

#endif // _EP_VIEWPORT_HPP
