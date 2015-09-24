#include "hal/driver.h"

#if EPWINDOW_DRIVER == EPDRIVER_NULL

#include "hal/haltimer.h"

HalTimer *HalTimer_Create(HalTimerType , uint32_t ,  HalTimerCallback *, void *)
{
  return nullptr;
}

void HalTimer_Destroy(HalTimer **)
{
}

#endif // EPWINDOW_DRIVER == EPDRIVER_NULL
