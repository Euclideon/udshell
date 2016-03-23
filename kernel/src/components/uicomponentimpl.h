#pragma once
#ifndef EPUICOMPONENTIMPL_H
#define EPUICOMPONENTIMPL_H

#include "ep/cpp/component/uicomponent.h"
#include "ep/cpp/internal/i/iuicomponent.h"

namespace ep {

inline Array<const PropertyInfo> UIComponent::GetProperties() const
{
  return{
    EP_MAKE_PROPERTY_RO(UIHandle, "Platform specific UI handle", nullptr, 0)
  };
}

class UIComponentImpl : public BaseImpl<UIComponent, IUIComponent>
{
public:
  UIComponentImpl(Component *pInstance, Variant::VarMap initParams)
    : ImplSuper(pInstance)
  {
  }
};


class UIComponentGlue final : public UIComponent
{
public:
  UIComponentGlue(const ComponentDesc *_pType, Kernel *_pKernel, SharedString _uid, ComponentRef _spInstance, Variant::VarMap initParams)
    : UIComponent(_pType, _pKernel, _uid, initParams), spInstance(_spInstance)
  {
  }

protected:
  ComponentRef spInstance;
};

} // namespace ep

#endif // EPUICOMPONENTIMPL_H
