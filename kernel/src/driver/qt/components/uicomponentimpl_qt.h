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
    return spQObject;
  }

  QObjectComponentRef spQObject = nullptr;
};


class UIComponentGlue final : public ep::UIComponent
{
public:
  UIComponentGlue(const ep::ComponentDesc *_pType, ep::Kernel *_pKernel, ep::SharedString _uid, ep::ComponentRef _spInstance, ep::Variant::VarMap initParams)
    : UIComponent(_pType, _pKernel, _uid, initParams)
  {
    GetImpl<QtUIComponentImpl>()->spQObject = ep::shared_pointer_cast<QObjectComponent>(_spInstance);
  }
};

} // namespace qt

#endif // EP_UICOMPONENTIMPL_QT_H
