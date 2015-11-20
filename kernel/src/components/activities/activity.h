#pragma once
#ifndef EP_ACTIVITY_H
#define EP_ACTIVITY_H

#include "components/component.h"

namespace kernel {

PROTOTYPE_COMPONENT(Activity);
SHARED_CLASS(UIComponent);

class Activity : public Component
{
  EP_DECLARE_COMPONENT(Activity, Component, EPKERNEL_PLUGINVERSION, "Activity desc...")
public:

  UIComponentRef GetUI() const { return ui; }

  virtual void Activate() {}
  virtual void Deactivate() {}

  Variant Save() const override { return Variant(); }

protected:
  Activity(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, InitParams initParams);

  UIComponentRef ui = nullptr;

  static Array<const PropertyInfo> GetProperties()
  {
    return{
      EP_MAKE_PROPERTY_RO(UI, "The top level UI compoment for this activity", nullptr, 0),
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

} //namespace kernel

#endif // EP_ACTIVITY_H
