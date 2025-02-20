#include "driver.h"

#if EPWINDOW_DRIVER == EPDRIVER_WIN32

#include "hal/timer.h"

struct HalTimerWin32 : public HalTimer
{
  HalTimerCallback *pCallBack;
  void *pUserData;
  HWND hwnd;

  UINT_PTR timer;
};

static void CALLBACK Win32Callback(HWND , UINT , UINT_PTR idEvent, DWORD )
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
  HalTimerWin32 *pTimer = epAllocType(HalTimerWin32, 1, epAF_Zero);
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
    epFree(pTimer);

  return pTimer;
}


void HalTimer_Destroy(HalTimer **ppTimer)
{
  if (ppTimer && *ppTimer)
  {
    HalTimerWin32 *pTimer = (HalTimerWin32*)*ppTimer;
    KillTimer(pTimer->hwnd, pTimer->timer);
    epFree((*ppTimer));
  }
}

#endif // EPWINDOW_DRIVER == EPDRIVER_WIN32

#if EPSYSTEM_DRIVER == EPDRIVER_WIN32

#include "hal/timer.h"

double epPerformanceCounter()
{
  LARGE_INTEGER counter, freq;
  QueryPerformanceCounter(&counter);
  QueryPerformanceFrequency(&freq);

  return (double)counter.QuadPart / freq.QuadPart;
}

#else
EPEMPTYFILE
#endif // EPSYSTEM_DRIVER == EPDRIVER_WIN32
