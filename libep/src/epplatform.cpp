#include "libep_internal.h"
#include "ep/cpp/platform.h"
#include <stdio.h>
#include "ep/cpp/plugin.h"

extern "C" {

void epInternalInit() epweak;

#if defined(EP_COMPILER_VISUALC)
void epInternalInitWeak() { }
# if defined(EP_ARCH_X86)
#   pragma comment(linker, "/alternatename:_epInternalInit=_epInternalInitWeak")
# else
#   pragma comment(linker, "/alternatename:epInternalInit=epInternalInitWeak")
# endif
#else
void epInternalInit() { }
#endif // defined(EP_COMPILER_VISUALC)

} // extern "C"

namespace ep {
namespace internal {

bool gUnitTesting = false;

MutableString256 assertBuffer;

} // namespace internal

void AssertFailed(ep::String condition, ep::String message, ep::String file, int line)
{
  // TODO: Agree on formatting of assets
  if (ep::internal::gUnitTesting)
  {
    fprintf(stderr, "%.*s", (int)message.length, message.ptr);
    exit(EXIT_FAILURE);
  }
  else
  {
    ep::MutableString256 t(ep::Format, "ASSERT FAILED : {0}\n{1}\n{2}({3})", condition, message, file, line);
    epDebugWrite(t.ptr);
  }
}

} // namespace ep

// Plugin API API
extern "C" {

void epAssertFailed(const char *condition, const char *message, const char *file, int line)
{
  ep::AssertFailed(condition, message, file, line);
}

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

  size_t len = ep::internal::epvscprintf(format, args) + 1;
  char *pBuffer = (char*)alloca(len + 1);
  len = ep::internal::epvsnprintf(pBuffer, len, format, args);
  va_end(args);

  epDebugWrite(pBuffer);
}

void *_epAlloc(size_t size, epAllocationFlags flags EP_IF_MEMORY_DEBUG(const char * pFile, int line))
{
#if __EP_MEMORY_DEBUG__
  const char *_pFile = pFile;
  int _line = line;
#else
  const char *_pFile = nullptr;
  int _line = 0;
#endif

  if (ep::s_pInstance)
  {
    return ep::s_pInstance->pAlloc(size, flags, _pFile, _line);
  }
  else
  {
    epInternalInit();
    if (ep::s_pInstance)
      return ep::s_pInstance->pAlloc(size, flags, _pFile, _line);
  }
  return nullptr;
}

void _epFree(void *pMemory)
{
  if (ep::s_pInstance)
  {
    ep::s_pInstance->pFree(pMemory);
  }
  else
  {
    epInternalInit();
    if (ep::s_pInstance)
      ep::s_pInstance->pFree(pMemory);
  }
}

#if EP_DEBUG
# if !defined(EP_WINDOWS)
#   if defined(EP_COMPILER_GCC)
#     include <sys/stat.h>
#     include <fcntl.h>
#     include <unistd.h>

      int IsDebuggerPresent(void)
      {
        char buf[1024] = { 0 };
        int debugger_present = 0;

        int status_fd = open("/proc/self/status", O_RDONLY);
        if (status_fd == -1)
            return 0;

        ssize_t num_read = read(status_fd, buf, sizeof(buf) - 1);

        if (num_read > 0)
        {
          const char TracerPid[] = "TracerPid:";
          char *tracer_pid = strstr(buf, TracerPid);
          if (tracer_pid)
              debugger_present = !!atoi(tracer_pid + sizeof(TracerPid) - 1);
        }
        return debugger_present;
      }
#   else
      int IsDebuggerPresent(void) { return 0; }
#   endif // defined(EP_COMPILER_GCC)

# endif // !defined(EP_WINDOWS)
#endif // EP_DEBUG

} // extern "C"
