#pragma once
#ifndef EP_HALTIMER_H
#define EP_HALTIMER_H

#include "ep/epplatform.h"

struct HalTimer;

enum HalTimerType
{
  Interval = 0,
  CountDown
};

struct HalTimer
{
  uint32_t duration;
  HalTimerType timerType;
  size_t elapsedCount;
};

typedef void (HalTimerCallback)(HalTimer *pTimer, void *pUserData);

HalTimer *HalTimer_Create(HalTimerType tt, uint32_t duration,  HalTimerCallback *pCallBack, void *pUserData);
void HalTimer_Destroy(HalTimer **ppTimer);

double epPerformanceCounter();

#endif // EP_HALTIMER_H
