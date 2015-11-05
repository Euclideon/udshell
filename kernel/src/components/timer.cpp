#include "components/timer.h"
#include "components/datasource.h"
#include "kernel.h"

namespace ep
{

static CPropertyDesc props[] =
{
  {
    {
      "duration", // id
      "Duration", // displayName
      "The duration for the timer", // description
    },

     &Timer::GetDuration,
     nullptr
  },

  {
    {
      "gettimertype", // id
      "Get Timer Type", // displayName
      "Gets the Timer Type", // description
    },

    &Timer::GetTimerType,
    nullptr
  },
};

static CMethodDesc methods[] =
{
  {
    {
      "setinterval", // id
      "Sets the Duration and the Timer Type to Interval", // description
    },
    &Timer::SetInterval  // method
  },

  {
    {
      "setcountdown", // id
      "Sets the Duration and the Timer Type to CountDown", // description
    },
    &Timer::SetCountDown  // method
  },

  {
    {
      "reset", // id
      "Resets the Timer", // description
    },
    &Timer::Reset  // method
  },

};

static CEventDesc events[] =
{
  {
    {
      "event", // id
      "Event", // displayName
      "Timer Event", // description
    },
    &Timer::Elapsed
  }
};

ComponentDesc Timer::descriptor =
{
  &Component::descriptor, // pSuperDesc

  EPSHELL_APIVERSION, // epVersion
  EPSHELL_PLUGINVERSION, // pluginVersion

  "timer", // id
  "Timer", // displayName
  "Timer ", // description

  Slice<CPropertyDesc>(props, UDARRAYSIZE(props)), // properties
  nullptr,
  Slice<CEventDesc>(events, UDARRAYSIZE(events))
};


Timer::Timer(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, InitParams initParams)
  : Component(pType, pKernel, uid, initParams), pTimer(nullptr)
{
  const Variant intervalVar = initParams["duration"];

  uint32_t duration = intervalVar.as<uint32_t>();
  if (!intervalVar.is(Variant::Type::Int) ||!duration)
  {
    throw epR_InvalidParameter;
  }

  TimerType timerType = TimerType::Interval;
  const Variant typeVar = initParams["timertype"];
  if (typeVar.is(Variant::Type::String))
  {
    String typeStr = typeVar.as<String>();
    if (typeStr.eqIC("Interval"))
      timerType = TimerType::Interval;
    else if (typeStr.eqIC("CountDown"))
      timerType = TimerType::CountDown;
    else
      throw epR_InvalidParameter;
  }

  pTimer = HalTimer_Create(HalTimerType(timerType.v), duration, TimerCallback, this);
  if (!pTimer)
    throw epR_Failure;
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


void Timer::MessageCallback(Kernel*)
{
  Elapsed.Signal();
}


void Timer::TimerCallback(HalTimer *pTimer, void *pParam)
{
  Timer* pThis = (Timer*)pParam;
  pThis->pKernel->DispatchToMainThread(MakeDelegate(pThis, &Timer::MessageCallback));
}

} // namespace ep
