#include "components/timer.h"
#include "ep/cpp/kernel.h"

namespace ep {

Array<const PropertyInfo> Timer::GetProperties() const
{
  return{
    EP_MAKE_PROPERTY_RO(Duration, "The duration for the timer", nullptr, 0),
    EP_MAKE_PROPERTY_RO(TimerType, "Gets the Timer Type", nullptr, 0),
  };
}
Array<const MethodInfo> Timer::GetMethods() const
{
  return{
    EP_MAKE_METHOD(SetInterval, "Sets the Duration and the Timer Type to Interval"),
    EP_MAKE_METHOD(SetCountDown, "Sets the Duration and the Timer Type to CountDown"),
    EP_MAKE_METHOD(Reset, "Resets the Timer"),
  };
}
Array<const EventInfo> Timer::GetEvents() const
{
  return{
    EP_MAKE_EVENT(Elapsed, "Timer Event"),
  };
}

Timer::Timer(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
  : Component(pType, pKernel, uid, initParams), pTimer(nullptr)
{
  const Variant intervalVar = *initParams.get("duration");

  uint32_t duration = intervalVar.as<uint32_t>();
  if (!intervalVar.is(Variant::Type::Int) || !duration)
  {
    EPTHROW_ERROR(epR_InvalidArgument, "Invalid 'duration'");
  }

  TimerType timerType = TimerType::Interval;
  const Variant typeVar = *initParams.get("timertype");
  if (typeVar.is(Variant::Type::String))
  {
    String typeStr = typeVar.as<String>();
    if (typeStr.eqIC("Interval"))
      timerType = TimerType::Interval;
    else if (typeStr.eqIC("CountDown"))
      timerType = TimerType::CountDown;
    else
      EPTHROW_ERROR(epR_InvalidArgument, "Invalid 'timertype'");
  }

  pTimer = HalTimer_Create(HalTimerType(timerType.v), duration, TimerCallback, this);
  if (!pTimer)
    EPTHROW_ERROR(epR_Failure, "Failed to create timer");
}


Timer::~Timer()
{
  HalTimer_Destroy(&pTimer);
}

void Timer::SetTimer(uint32_t d, TimerType tt)
{
  HalTimer_Destroy(&pTimer);
  pTimer = HalTimer_Create(HalTimerType(tt.v), d, TimerCallback, this);
}


void Timer::MessageCallback()
{
  Elapsed.Signal();
  DecRef();
}


void Timer::TimerCallback(HalTimer *pTimer, void *pParam)
{
  Timer* pThis = (Timer*)pParam;
  pThis->IncRef();
  pThis->pKernel->DispatchToMainThread(MakeDelegate(pThis, &Timer::MessageCallback));
}

} // namespace ep
