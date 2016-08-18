#include "driver.h"

#if EPSYSTEM_DRIVER == EPDRIVER_NULL

#include "hal/library.h"

bool epLibrary_Open(epLibrary *pLibrary, const char *filename)
{
  return false;
}

bool epLibrary_Close(epLibrary library)
{
  return false;
}

void *epLibrary_GetFunction(epLibrary library, const char *funcName)
{
  return nullptr;
}

char *epLibrary_GetLastError()
{
  return nullptr;
}

#else
EPEMPTYFILE
#endif // EPSYSTEM_DRIVER == EPDRIVER_NULL
