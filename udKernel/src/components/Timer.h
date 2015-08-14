#pragma once
#if !defined(_UD_TIMER_H)
#define _UD_TIMER_H

#include "component.h"
#include "components/stream.h"
#include "resources/resource.h"
#include "SDL2/SDL_timer.h"
#include "hal/haltimer.h"

namespace ud
{

PROTOTYPE_COMPONENT(Timer);

class Timer : public Component
{
public:
  UD_COMPONENT(Timer);

  UD_ENUM(TimerType,
    Interval,
    CountDown
    );

  udEvent<> Event;

  uint32_t GetDuration() const { return pTimer ? pTimer->duration : 0; };
  TimerType GetTimerType() const { return pTimer ? TimerType(pTimer->timerType) : TimerType::Interval;}

  void Reset() { if (pTimer) SetTimer(pTimer->duration, TimerType(pTimer->timerType)); }

  void SetInterval(uint32_t interval) { SetTimer(interval, TimerType::Interval); }
  void SetCountDown(uint32_t countDown) { SetTimer(countDown, TimerType::CountDown); }

  void MessageCallback(Kernel *pKernel);

private:
  Timer(const ComponentDesc *pType, Kernel *pKernel, udRCString uid, udInitParams initParams);

  static void TimerCallback(HalTimer *pTimer, void *pParam);
  void SetTimer(uint32_t duration, TimerType tt);

  HalTimer *pTimer;
};

}

#endif // _UD_TIMER_H
