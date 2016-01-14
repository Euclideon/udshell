#pragma once
#ifndef EPUICOMPONENTIMPL_H
#define EPUICOMPONENTIMPL_H

#include "ep/cpp/component/uicomponent.h"
#include "ep/cpp/internal/i/iuicomponent.h"

namespace ep {

class UIComponentImpl : public BaseImpl<UIComponent, IUIComponent>
{
public:
  UIComponentImpl(Component *pInstance, Variant::VarMap initParams);

  Variant GetUIHandle() const override final;

protected:
  ~UIComponentImpl() { DestroyInternal(); }

  void InitComplete() override;

  epResult CreateInternal(Variant::VarMap initParams);
  void DestroyInternal();
};

} // namespace ep

#endif // EPUICOMPONENTIMPL_H
