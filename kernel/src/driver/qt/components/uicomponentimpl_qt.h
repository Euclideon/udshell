#pragma once
#ifndef EP_UICOMPONENTIMPL_QT_H
#define EP_UICOMPONENTIMPL_QT_H

#include "components/uicomponentimpl.h"
#include "driver/qt/components/qobjectcomponent_qt.h"

namespace qt {

class QtUIComponentImpl : public ep::UIComponentImpl
{
public:
  QtUIComponentImpl(ep::Component *pInstance, ep::Variant::VarMap initParams)
    : ep::UIComponentImpl(pInstance, initParams)
  {
  }

  ep::Variant GetUIHandle() const override final {
    return GetKernel()->CreateComponent<qt::QObjectComponent>({ { "object", (int64_t)(size_t)pInstance->GetUserData() } });
  }
};

} // namespace qt

#endif // EP_UICOMPONENTIMPL_QT_H
