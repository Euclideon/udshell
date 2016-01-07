#include "hal/driver.h"

#if EPSYSTEM_DRIVER == EPDRIVER_POSIX

#include "hal/haltimer.h"

#include <time.h>

double epPerformanceCounter()
{
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return ts.tv_sec + (ts.tv_nsec / 1000000000.0);
}

#else
EPEMPTYFILE
#endif // EPSYSTEM_DRIVER == EPDRIVER_POSIX
