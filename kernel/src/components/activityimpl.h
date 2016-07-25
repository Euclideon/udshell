#pragma once
#ifndef EPACTIVITYIMPL_H
#define EPACTIVITYIMPL_H

#include "ep/cpp/component/activity.h"
#include "ep/cpp/internal/i/iactivity.h"
#include "components/uicomponentimpl.h"

namespace ep {

inline Array<const PropertyInfo> Activity::GetProperties() const
{
  return{
    EP_MAKE_PROPERTY("ui", GetUI, SetUI, "The top level UI component for this activity", nullptr, 0),
  };
}
inline Array<const MethodInfo> Activity::GetMethods() const
{
  return{
    EP_MAKE_METHOD(Activate, "Set as the active activity"),
    EP_MAKE_METHOD(Deactivate, "Unset as the active activity"),
  };
}

class ActivityImpl : public BaseImpl<Activity, IActivity>
{
public:
  ActivityImpl(Component *pInstance, Variant::VarMap initParams)
    : ImplSuper(pInstance)
  {
  }

  UIComponentRef GetUI() const override final { return ui; }
  void SetUI(UIComponentRef _ui) override final { ui = _ui; }

  void Activate() override final {}
  void Deactivate() override final {}

  Variant Save() const override final { return pInstance->Super::Save(); }

protected:
  UIComponentRef ui = nullptr;
};

} //namespace ep

#endif // EPACTIVITYIMPL_H
