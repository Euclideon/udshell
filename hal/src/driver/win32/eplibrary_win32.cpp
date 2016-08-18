#include "driver.h"

#if EPSYSTEM_DRIVER == EPDRIVER_WIN32

#include "hal/library.h"

static_assert(sizeof(HMODULE) == sizeof(void*), "Types not equal");

bool epLibrary_Open(epLibrary *pLibrary, const char *filename)
{
  // Convert UTF-8 to UTF-16 -- TODO use UD helper functions or add some to hal?
  int len = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, filename, -1, nullptr, 0);
  wchar_t *widePath = (wchar_t*)alloca(sizeof(wchar_t) * len);
  if (MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, filename, -1, widePath, len) == 0)
    return false;

  // try and load library
  *pLibrary = (epLibrary)LoadLibraryW(widePath);
  return (*pLibrary != 0);
}

bool epLibrary_Close(epLibrary library)
{
  return (FreeLibrary((HMODULE)library) == 0);
}

void *epLibrary_GetFunction(epLibrary library, const char *funcName)
{
  return GetProcAddress((HMODULE)library, funcName);
}

char *epLibrary_GetLastError()
{
  static char buffer[128] = {0};
  FormatMessageA(FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM,
    nullptr, GetLastError(), 0, buffer, sizeof(buffer) / sizeof(char), nullptr);
  return buffer;
}

#else
EPEMPTYFILE
#endif // EPSYSTEM_DRIVER == EPDRIVER_WIN32
