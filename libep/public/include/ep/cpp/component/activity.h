#pragma once
#ifndef EP_ACTIVITY_H
#define EP_ACTIVITY_H

#include "ep/cpp/component/component.h"
#include "ep/cpp/internal/i/iactivity.h"

namespace ep {

SHARED_CLASS(Activity);

class Activity : public Component, public IActivity
{
  EP_DECLARE_COMPONENT_WITH_IMPL(Activity, IActivity, Component, EPKERNEL_PLUGINVERSION, "Activity desc...")
public:

  ComponentRef GetUI() const override final;
  void SetUI(ComponentRef ui) override final;

  void Activate() override;
  void Deactivate() override;

  Variant Save() const override;

protected:
  Activity(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams);

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
