#include "components/timer.h"
#include "ep/cpp/component/datasource/datasource.h"
#include "kernel.h"

namespace ep {

Timer::Timer(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
  : Component(pType, pKernel, uid, initParams), pTimer(nullptr)
{
  const Variant intervalVar = *initParams.Get("duration");

  uint32_t duration = intervalVar.as<uint32_t>();
  if (!intervalVar.is(Variant::Type::Int) || !duration)
  {
    EPTHROW_ERROR(epR_InvalidArgument, "Invalid 'duration'");
  }

  TimerType timerType = TimerType::Interval;
  const Variant typeVar = *initParams.Get("timertype");
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


void Timer::MessageCallback(ep::Kernel*)
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
