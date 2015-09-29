#include "hal/driver.h"

#if EPSYSTEM_DRIVER == EPDRIVER_POSIX

#include "hal/haltimer.h"

double epPerformanceCounter()
{
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return ts.tv_sec + (ts.tv_nsec / 1000000000.0);
}

#endif // EPSYSTEM_DRIVER == EPDRIVER_POSIX
