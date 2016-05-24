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

  uint32_t GetDuration() const override final { return pTimer ? pTimer->duration : 0; };
  TimerType GetTimerType() const override final { return pTimer ? TimerType(pTimer->timerType) : TimerType::Interval;}

  void Reset() override final { if (pTimer) SetTimer(pTimer->duration, TimerType(pTimer->timerType)); }

  void SetInterval(uint32_t interval) override final { SetTimer(interval, TimerType::Interval); }
  void SetCountDown(uint32_t countDown) override final { SetTimer(countDown, TimerType::CountDown); }

private:
  static void TimerCallback(HalTimer *pTimer, void *pParam);
  void SetTimer(uint32_t duration, TimerType tt);

  void MessageCallback();

  HalTimer *pTimer;
};

} // namespace ep

#endif // _EP_TIMERIMPL_HPP
