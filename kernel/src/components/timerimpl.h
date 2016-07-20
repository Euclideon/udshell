#pragma once
#if !defined(_EP_TIMERIMPL_HPP)
#define _EP_TIMERIMPL_HPP

#include "ep/cpp/component/timer.h"
#include "hal/timer.h"

namespace ep {

SHARED_CLASS(Timer);

class TimerImpl : public BaseImpl<Timer, ITimer>
{
public:
  TimerImpl(Component *pInstance, Variant::VarMap initParams);
  ~TimerImpl();

  double GetDuration() const override final { return pTimer ? (double)pTimer->duration * 0.001 : 0.0; };
  TimerType GetTimerType() const override final { return pTimer ? TimerType(pTimer->timerType) : TimerType::Interval;}

  void Reset() override final { if (pTimer) SetTimer(pTimer->duration, TimerType(pTimer->timerType)); }

  void BeginInterval(double interval) override final { SetTimer(interval, TimerType::Interval); }
  void BeginCountdown(double duration) override final { SetTimer(duration, TimerType::Countdown); }

private:
  static void TimerCallback(HalTimer *pTimer, void *pParam);
  void SetTimer(double duration, TimerType tt);

  void MessageCallback();

  HalTimer *pTimer = nullptr;
};

} // namespace ep

#endif // _EP_TIMERIMPL_HPP
