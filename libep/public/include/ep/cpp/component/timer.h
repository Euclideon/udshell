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

  //! Get the currently configured timer duration.
  //! \return The duration of the timer in seconds.
  //! \see GetTimerType
  double GetDuration() const { return pImpl->GetDuration(); };

  //! Get the currently configured timer type.
  //! \return The type of the timer.
  //! \see GetDuration
  TimerType GetTimerType() const { return pImpl->GetTimerType(); }

  //! Reset the timer using the currently configured settings.
  //! \return None.
  void Reset() { pImpl->Reset(); }

  //! Begin an interval counter. The \c Elapsed signal will trigger each \a interval seconds.
  //! \param interval The timer interval in seconds.
  //! \return None.
  //! \see BeginCountdown
  void BeginInterval(double interval) { pImpl->BeginInterval(interval); }

  //! Begin a countdown. The \c Elapsed signal will trigger after the countdown completes.
  //! \param duration The countdown duration in seconds.
  //! \return None.
  //! \see BeginInterval
  void BeginCountdown(double duration) { pImpl->BeginCountdown(duration); }

  //! Event that triggers when the timer elapses.
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
