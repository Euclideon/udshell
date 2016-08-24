#include "driver.h"

#if EPSYSTEM_DRIVER == EPDRIVER_POSIX

#include "hal/library.h"
#include <dlfcn.h>

bool epLibrary_Open(epLibrary *pLibrary, const char *filename)
{
  // try and load library
  *pLibrary = (epLibrary)dlopen(filename, RTLD_NOW);
  return (*pLibrary != 0);
}

bool epLibrary_Close(epLibrary library)
{
  return dlclose(library);
}

void *epLibrary_GetFunction(epLibrary library, const char *funcName)
{
  return dlsym(library, funcName);
}

char *epLibrary_GetLastError()
{
  return dlerror();
}

#else
EPEMPTYFILE
#endif // EPSYSTEM_DRIVER == EPDRIVER_POSIX
