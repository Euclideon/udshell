#pragma once
#if !defined(_EP_ITIMER_HPP)
#define _EP_ITIMER_HPP

#include "ep/cpp/component/component.h"

namespace ep {

EP_ENUM(TimerType,
  Interval,
  Countdown
);

class ITimer
{
public:
  virtual double GetDuration() const = 0;
  virtual TimerType GetTimerType() const = 0;

  virtual void Reset() = 0;

  virtual void BeginInterval(double interval) = 0;
  virtual void BeginCountdown(double countDown) = 0;
};

} // namespace ep

#endif // _EP_ITIMER_HPP
