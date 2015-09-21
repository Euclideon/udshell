#include "hal/driver.h"

#if UDWINDOW_DRIVER == UDDRIVER_NULL

#include "hal/haltimer.h"

HalTimer *HalTimer_Create(HalTimerType , uint32_t ,  HalTimerCallback *, void *)
{
  return nullptr;
}

void HalTimer_Destroy(HalTimer **)
{
}

#endif // UDWINDOW_DRIVER == UDDRIVER_NULL
