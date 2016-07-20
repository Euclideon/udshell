#pragma once
#if !defined(_EP_TIMER_HPP)
#define _EP_TIMER_HPP

#include "ep/cpp/component/component.h"
#include "ep/cpp/internal/i/itimer.h"

namespace ep {

SHARED_CLASS(Timer);

class Timer : public Component
{
  EP_DECLARE_COMPONENT_WITH_IMPL(ep, Timer, ITimer, Component, EPKERNEL_PLUGINVERSION, "Timer desc...", 0)
public:

  double GetDuration() const { return pImpl->GetDuration(); };
  TimerType GetTimerType() const { return pImpl->GetTimerType(); }

  void Reset() { pImpl->Reset(); }

  void BeginInterval(double interval) { pImpl->BeginInterval(interval); }
  void BeginCountdown(double duration) { pImpl->BeginCountdown(duration); }

  Event<> Elapsed;

private:
  Timer(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
    : Component(pType, pKernel, uid, initParams)
  {
    pImpl = CreateImpl(initParams);
  }

  Array<const PropertyInfo> GetProperties() const;
  Array<const MethodInfo> GetMethods() const;
  Array<const EventInfo> GetEvents() const;
};

} // namespace ep

#endif // _EP_TIMER_HPP
