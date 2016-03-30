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

private:
  Array<const PropertyInfo> GetProperties() const;
  Array<const MethodInfo> GetMethods() const;
};

} //namespace ep

#endif // EP_ACTIVITY_H
