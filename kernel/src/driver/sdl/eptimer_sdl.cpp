#include "hal/driver.h"

#if EPWINDOW_DRIVER == EPDRIVER_SDL

#include "hal/haltimer.h"
#include "SDL2/SDL_timer.h"

struct HalTimerSDL : public HalTimer
{
  HalTimerCallback *pCallBack;
  void *pUserData;

  SDL_TimerID timerID;
};


static uint32_t SDLCallback(uint32_t duration, void *pUserData)
{
  HalTimerSDL *pTimer = (HalTimerSDL*)pUserData;
  pTimer->pCallBack(pTimer, pTimer->pUserData);

  pTimer->elapsedCount++;
  if (pTimer->timerType == CountDown)
    return 0;

  return duration;
}


HalTimer *HalTimer_Create(HalTimerType tt, uint32_t duration,  HalTimerCallback *pCallBack, void *pUserData)
{
  HalTimerSDL *pTimer = epAllocType(HalTimerSDL, 1, epAF_Zero);
  if (!pTimer)
    return nullptr;

  pTimer->duration = duration;
  pTimer->timerType = tt;
  pTimer->pCallBack = pCallBack;
  pTimer->pUserData = pUserData;

  pTimer->elapsedCount = 0;
  pTimer->timerID = SDL_AddTimer(duration, SDLCallback, pTimer);

  if (pTimer->timerID == 0)
    epFree(pTimer);

  return pTimer;
}


void HalTimer_Destroy(HalTimer **ppTimer)
{
  if (ppTimer && *ppTimer)
  {
    HalTimerSDL *pTimer = (HalTimerSDL*)*ppTimer;
    SDL_RemoveTimer(pTimer->timerID);
    epFree((*ppTimer));
  }
}

#endif // EPWINDOW_DRIVER == EPDRIVER_SDL
