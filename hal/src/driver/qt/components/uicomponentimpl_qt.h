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
    getImpl<QtUIComponentImpl>()->spQObject = ep::shared_pointer_cast<QObjectComponent>(_spInstance);
  }

  /*const ep::PropertyDesc *GetPropertyDesc(String _name, EnumerateFlags enumerateFlags = 0) const override final { return spInstance->GetPropertyDesc(_name, enumerateFlags); }
  const ep::MethodDesc *GetMethodDesc(String _name, EnumerateFlags enumerateFlags = 0) const override final { return spInstance->GetMethodDesc(_name, enumerateFlags); }
  const ep::EventDesc *GetEventDesc(String _name, EnumerateFlags enumerateFlags = 0) const override final { return spInstance->GetEventDesc(_name, enumerateFlags); }
  const ep::StaticFuncDesc *GetStaticFuncDesc(String _name, EnumerateFlags enumerateFlags = 0) const override final { return spInstance->GetStaticFuncDesc(_name, enumerateFlags); }*/
};

} // namespace qt

#endif // EP_UICOMPONENTIMPL_QT_H
