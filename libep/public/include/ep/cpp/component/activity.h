#pragma once
#ifndef EP_ACTIVITY_H
#define EP_ACTIVITY_H

#include "ep/cpp/component/uicomponent.h"
#include "ep/cpp/internal/i/iactivity.h"

namespace ep {

SHARED_CLASS(Activity);

class Activity : public Component
{
  EP_DECLARE_COMPONENT_WITH_IMPL(ep, Activity, IActivity, Component, EPKERNEL_PLUGINVERSION, "Activity desc...", 0)
public:

  virtual void activate() { pImpl->Activate(); }
  virtual void deactivate() { pImpl->Deactivate(); }

  UIComponentRef getUI() const { return pImpl->GetUI(); }
  void setUI(UIComponentRef ui) { pImpl->SetUI(ui); }

  Variant save() const override { return pImpl->Save(); }

protected:
  Activity(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
    : Component(pType, pKernel, uid, initParams)
  {
    pImpl = createImpl(initParams);
  }

private:
  Array<const PropertyInfo> getProperties() const;
  Array<const MethodInfo> getMethods() const;
};

} //namespace ep

#endif // EP_ACTIVITY_H
