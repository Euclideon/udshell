#pragma once
#ifndef HALTIMER_H
#define HALTIMER_H

#include "udPlatform.h"
#include "udResult.h"

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

#endif // HALTIMER_H
