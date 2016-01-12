#pragma once
#ifndef EPACTIVITYIMPL_H
#define EPACTIVITYIMPL_H

#include "ep/cpp/component/activity.h"
#include "ep/cpp/internal/i/iactivity.h"

namespace ep {

SHARED_CLASS(UIComponent);

class ActivityImpl : public BaseImpl<Activity, IActivity>
{
public:
  ActivityImpl(Component *pInstance, Variant::VarMap initParams)
    : Super(pInstance)
  {
  }

  ComponentRef GetUI() const override final;
  void SetUI(ComponentRef ui) override final;

  void Activate() override;
  void Deactivate() override;

  Variant Save() const override;

protected:
  ~ActivityImpl();

  UIComponentRef ui = nullptr;
};

} //namespace ep

#endif // EPACTIVITYIMPL_H
