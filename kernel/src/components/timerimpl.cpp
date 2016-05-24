#include "components/timerimpl.h"
#include "ep/cpp/kernel.h"
#include "hal/timer.h"

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

TimerImpl::TimerImpl(Component *pInstance, Variant::VarMap initParams)
  : ImplSuper(pInstance)
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


TimerImpl::~TimerImpl()
{
  HalTimer_Destroy(&pTimer);
}

void TimerImpl::SetTimer(uint32_t d, TimerType tt)
{
  HalTimer_Destroy(&pTimer);
  pTimer = HalTimer_Create(HalTimerType(tt.v), d, TimerCallback, this);
}


void TimerImpl::MessageCallback()
{
  pInstance->Elapsed.Signal();
  pInstance->DecRef();
}


void TimerImpl::TimerCallback(HalTimer *pTimer, void *pParam)
{
  TimerImpl* pThis = (TimerImpl*)pParam;
  pThis->pInstance->IncRef();
  pThis->GetKernel()->DispatchToMainThread(MakeDelegate(pThis, &TimerImpl::MessageCallback));
}

} // namespace ep
