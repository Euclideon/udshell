#include "hal/driver.h"
#include "hal/haltimer.h"

#if UDINPUT_DRIVER == UDDRIVER_WIN32

struct HalTimerWin32 : public HalTimer
{
  HalTimerCallback *pCallBack;
  void *pUserData;
  HWND hwnd;

  UINT_PTR timer;
};

static void Win32Callback(HWND , UINT , UINT_PTR idEvent, DWORD )
{
  HalTimerWin32 *pTimer = (HalTimerWin32*)idEvent;
  pTimer->pCallBack(pTimer, pTimer->pUserData);

  pTimer->elapsedCount++;
  if (pTimer->timerType == CountDown)
  {
    KillTimer(pTimer->hwnd, pTimer->timer);
  }
}


HalTimer *HalTimer_Create(HalTimerType tt, uint32_t duration,  HalTimerCallback *pCallBack, void *pUserData)
{
  HalTimerWin32 *pTimer = udAllocType(HalTimerWin32, 1, udAF_Zero);
  if (!pTimer)
    return nullptr;

  pTimer->duration = duration;
  pTimer->timerType = tt;
  pTimer->pCallBack = pCallBack;
  pTimer->pUserData = pUserData;

  pTimer->elapsedCount = 0;
  pTimer->hwnd = GetActiveWindow();
  pTimer->timer = SetTimer(pTimer->hwnd, (UINT_PTR)pTimer, duration, Win32Callback);

  if (pTimer->timer == 0)
    udFree(pTimer);

  return pTimer;
}


void HalTimer_Destroy(HalTimer **ppTimer)
{
  if (ppTimer && *ppTimer)
  {
    HalTimerWin32 *pTimer = (HalTimerWin32*)*ppTimer;
    KillTimer(pTimer->hwnd, pTimer->timer);
    udFree((*ppTimer));
  }
}

#endif // UDINPUT_DRIVER == UDDRIVER_WIN32
