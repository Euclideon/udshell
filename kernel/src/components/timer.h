#pragma once
#if !defined(_EP_TIMER_H)
#define _EP_TIMER_H

#include "component.h"
#include "components/stream.h"
#include "resources/resource.h"
#include "hal/haltimer.h"

namespace ep
{

PROTOTYPE_COMPONENT(Timer);

class Timer : public Component
{
public:
  EP_COMPONENT(Timer);

  EP_ENUM(TimerType,
    Interval,
    CountDown
    );

  epEvent<> Event;

  uint32_t GetDuration() const { return pTimer ? pTimer->duration : 0; };
  TimerType GetTimerType() const { return pTimer ? TimerType(pTimer->timerType) : TimerType::Interval;}

  void Reset() { if (pTimer) SetTimer(pTimer->duration, TimerType(pTimer->timerType)); }

  void SetInterval(uint32_t interval) { SetTimer(interval, TimerType::Interval); }
  void SetCountDown(uint32_t countDown) { SetTimer(countDown, TimerType::CountDown); }

  void MessageCallback(Kernel *pKernel);

private:
  Timer(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, InitParams initParams);
  virtual ~Timer();

  static void TimerCallback(HalTimer *pTimer, void *pParam);
  void SetTimer(uint32_t duration, TimerType tt);

  HalTimer *pTimer;
};

}

#endif // _EP_TIMER_H
