#pragma once
#ifndef EP_ACTIVITY_H
#define EP_ACTIVITY_H

#include "ep/cpp/component/uicomponent.h"
#include "ep/cpp/internal/i/iactivity.h"

namespace ep {

SHARED_CLASS(Activity);

class Activity : public Component
{
  EP_DECLARE_COMPONENT_WITH_IMPL(Activity, IActivity, Component, EPKERNEL_PLUGINVERSION, "Activity desc...", 0)
public:

  virtual void Activate() { pImpl->Activate(); }
  virtual void Deactivate() { pImpl->Deactivate(); }

  UIComponentRef GetUI() const { return pImpl->GetUI(); }
  void SetUI(UIComponentRef ui) { pImpl->SetUI(ui); }

  Variant Save() const override { return pImpl->Save(); }

protected:
  Activity(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
    : Component(pType, pKernel, uid, initParams)
  {
    pImpl = CreateImpl(initParams);
  }

  static Array<const PropertyInfo> GetProperties()
  {
    return{
      EP_MAKE_PROPERTY(UI, "The top level UI compoment for this activity", nullptr, 0),
    };
  }

  static Array<const MethodInfo> GetMethods()
  {
    return{
      EP_MAKE_METHOD(Activate, "Set as the active activity"),
      EP_MAKE_METHOD(Deactivate, "Unset as the active activity"),
    };
  }
};

} //namespace ep

#endif // EP_ACTIVITY_H
