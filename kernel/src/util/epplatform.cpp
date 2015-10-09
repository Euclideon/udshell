#include "ep/epplatform.h"

#include "udPlatform.h"

namespace ep {
namespace internal {

bool gUnitTesting = false;
epMutableString256 assertBuffer;

void epAssertFailed(epString condition, epString message, epString file, int line)
{
  // TODO: Agree on formatting of assets
  if (gUnitTesting)
  {
    fprintf(stderr, message.toStringz());
    exit(EXIT_FAILURE);
  }
  else
  {
    epMutableString256 t; t.format("ASSERT FAILED : {0}\n{1}\n{2}({3})", condition, message, file, line);
    epDebugWrite(t.ptr);
  }
}

} // namespace internal
} // namespace ep

void epDebugWrite(const char *pString)
{
#ifdef EP_WINDOWS
  OutputDebugStringA(pString);
#else
  fprintf(stderr, "%s", pString);
#endif
}

void epDebugPrintf(const char *format, ...)
{
  va_list args;
  va_start(args, format);

#if defined(EP_COMPILER_VISUALC)
  size_t len = _vscprintf(format, args) + 1;
  char *pBuffer = (char*)alloca(len + 1);
  vsnprintf_s(pBuffer, len, len, format, args);
#else
  size_t len = vsprintf(nullptr, format, args) + 1;
  char *pBuffer = (char*)alloca(len + 1);
  vsnprintf(pBuffer, len, format, args);
#endif
  pBuffer[len] = 0;

  va_end(args);

  epDebugWrite(pBuffer);
}
