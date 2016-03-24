#pragma once
#if !defined(_EP_TIMER_H)
#define _EP_TIMER_H

#include "ep/cpp/component/component.h"
#include "hal/haltimer.h"

namespace ep {

EP_ENUM(TimerType,
  Interval,
  CountDown
);

SHARED_CLASS(Timer);

class Timer : public Component
{
  EP_DECLARE_COMPONENT(Timer, Component, EPKERNEL_PLUGINVERSION, "Timer desc...", 0)
public:

  Event<> Elapsed;

  uint32_t GetDuration() const { return pTimer ? pTimer->duration : 0; };
  TimerType GetTimerType() const { return pTimer ? TimerType(pTimer->timerType) : TimerType::Interval;}

  void Reset() { if (pTimer) SetTimer(pTimer->duration, TimerType(pTimer->timerType)); }

  void SetInterval(uint32_t interval) { SetTimer(interval, TimerType::Interval); }
  void SetCountDown(uint32_t countDown) { SetTimer(countDown, TimerType::CountDown); }

  void MessageCallback();

private:
  Timer(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams);
  virtual ~Timer();

  static void TimerCallback(HalTimer *pTimer, void *pParam);
  void SetTimer(uint32_t duration, TimerType tt);

  HalTimer *pTimer;

  Array<const PropertyInfo> GetProperties() const;
  Array<const MethodInfo> GetMethods() const;
  Array<const EventInfo> GetEvents() const;
};

} // namespace ep

#endif // _EP_TIMER_H
