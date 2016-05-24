#pragma once
#if !defined(_EP_ITIMER_HPP)
#define _EP_ITIMER_HPP

#include "ep/cpp/component/component.h"

namespace ep {

EP_ENUM(TimerType,
  Interval,
  CountDown
);

class ITimer
{
public:
  virtual uint32_t GetDuration() const = 0;
  virtual TimerType GetTimerType() const = 0;

  virtual void Reset() = 0;

  virtual void SetInterval(uint32_t interval) = 0;
  virtual void SetCountDown(uint32_t countDown) = 0;
};

} // namespace ep

#endif // _EP_ITIMER_HPP
