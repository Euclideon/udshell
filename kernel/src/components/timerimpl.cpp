#include "components/timerimpl.h"
#include "ep/cpp/kernel.h"
#include "hal/timer.h"

namespace ep {

Array<const PropertyInfo> Timer::getProperties() const
{
  return{
    EP_MAKE_PROPERTY_RO("duration", GetDuration, "The duration for the timer in seconds", nullptr, 0),
    EP_MAKE_PROPERTY_RO("timerType", GetTimerType, "The TimerType of this timer", nullptr, 0),
  };
}
Array<const MethodInfo> Timer::getMethods() const
{
  return{
    EP_MAKE_METHOD(BeginInterval, "Begins an interval counter triggering at each given interval"),
    EP_MAKE_METHOD(BeginCountdown, "Begins a countdown of the given duration"),
    EP_MAKE_METHOD(Reset, "Resets the Timer"),
  };
}
Array<const EventInfo> Timer::getEvents() const
{
  return{
    EP_MAKE_EVENT(Elapsed, "Elapsed Event"),
  };
}

TimerImpl::TimerImpl(Component *pInstance, Variant::VarMap initParams)
  : ImplSuper(pInstance)
{
  const Variant *pCountdown = initParams.get("countdown");
  const Variant *pInterval = initParams.get("interval");

  if (pCountdown && pInterval)
    LogWarning(1, "'countdown' and 'interval' both supplied to Timer initParams, 'interval' will be used.");

  if (pInterval)
    BeginInterval(pInterval->as<double>());
  else if (pCountdown)
    BeginCountdown(pCountdown->as<double>());
}


TimerImpl::~TimerImpl()
{
  if (pTimer)
    HalTimer_Destroy(&pTimer);
}

void TimerImpl::SetTimer(double d, TimerType tt)
{
  if (pTimer)
    HalTimer_Destroy(&pTimer);
  pTimer = HalTimer_Create(HalTimerType(tt.v), (uint32_t)(d * 1000.0), TimerCallback, this);
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
  pThis->GetKernel()->dispatchToMainThread(MakeDelegate(pThis, &TimerImpl::MessageCallback));
}

} // namespace ep
