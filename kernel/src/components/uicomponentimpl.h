#pragma once
#ifndef EPUICOMPONENTIMPL_H
#define EPUICOMPONENTIMPL_H

#include "ep/cpp/component/uicomponent.h"
#include "ep/cpp/internal/i/iuicomponent.h"
#include "components/componentimpl.h"

namespace ep {

inline Array<const PropertyInfo> UIComponent::getProperties() const
{
  return{
    EP_MAKE_PROPERTY_RO("uiHandle", getUIHandle, "Platform specific UI handle", nullptr, 0)
  };
}

class UIComponentImpl : public BaseImpl<UIComponent, IUIComponent>
{
public:
  UIComponentImpl(Component *pInstance, Variant::VarMap initParams)
    : ImplSuper(pInstance)
  {
  }

  void PostInit(void *pData) override final
  {
    pInstance->ep::Component::GetImpl<ep::ComponentImpl>()->SetUserData(pData);
  }
};

} // namespace ep

#endif // EPUICOMPONENTIMPL_H
