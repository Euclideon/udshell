#pragma once
#ifndef EP_ACTIVITY_H
#define EP_ACTIVITY_H

#include "components/component.h"

namespace ep
{
PROTOTYPE_COMPONENT(Activity);
SHARED_CLASS(UIComponent);

class Activity : public Component
{
public:
  EP_COMPONENT(Activity);

  UIComponentRef GetUI() const { return ui; }
  virtual void Activate() {}
  virtual void Deactivate() {}
  Variant Save() const override { return Variant(); }

protected:
  UIComponentRef ui = nullptr;

  Activity(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, InitParams initParams);
};

} //namespace ep
#endif // EP_ACTIVITY_H
