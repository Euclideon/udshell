#include "ep/cpp/platform.h"

#include "udPlatform.h"

#include <stdio.h>

namespace ep {
namespace internal {

bool gUnitTesting = false;

MutableString256 assertBuffer;

} // namespace internal
} // namespace ep

// Plugin API API
extern "C" {

void epAssertFailed(epString condition, epString message, epString file, int line)
{
  // TODO: Agree on formatting of assets
  if (internal::gUnitTesting)
  {
    fprintf(stderr, "%.*s", (int)message.length, message.ptr);
    exit(EXIT_FAILURE);
  }
  else
  {
    MutableString256 t(Format, "ASSERT FAILED : {0}\n{1}\n{2}({3})", (String&)condition, (String&)message, (String&)file, line);
    epDebugWrite(t.ptr);
  }
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

#if defined(EP_COMPILER_VISUALC)
  size_t len = _vscprintf(format, args) + 1;
  char *pBuffer = (char*)alloca(len + 1);
  vsnprintf_s(pBuffer, len, len, format, args);
#else
  size_t len = vsnprintf(nullptr, 0, format, args) + 1;
  char *pBuffer = (char*)alloca(len + 1);
  vsnprintf(pBuffer, len, format, args);
#endif
  pBuffer[len] = 0;

  va_end(args);

  epDebugWrite(pBuffer);
}


// HAX: memory functions from UD
#define EP_DEFAULT_ALIGNMENT (8)
void *_epAlloc(size_t size, epAllocationFlags flags IF_MEMORY_DEBUG(const char * pFile, int line))
{
#if defined(EP_COMPILER_VISUALC)
  void *pMemory = (flags & epAF_Zero) ? _aligned_recalloc(nullptr, size, 1, EP_DEFAULT_ALIGNMENT) : _aligned_malloc(size, EP_DEFAULT_ALIGNMENT);
#else
  void *pMemory = (flags & epAF_Zero) ? calloc(size, 1) : malloc(size);
#endif
  return pMemory;
}

void *_epAllocAligned(size_t size, size_t alignment, epAllocationFlags flags IF_MEMORY_DEBUG(const char * pFile, int line))
{
#if defined(EP_COMPILER_VISUALC)
  void *pMemory = (flags & epAF_Zero) ? _aligned_recalloc(nullptr, size, 1, alignment) : _aligned_malloc(size, alignment);
#elif EP_NACL
  void *pMemory = (flags & epAF_Zero) ? calloc(size, 1) : malloc(size);
#elif defined(__GNUC__)
  if (alignment < sizeof(size_t))
    alignment = sizeof(size_t);
  void *pMemory;
  int err = posix_memalign(&pMemory, alignment, size + alignment);
  if (err != 0)
    return nullptr;

  if (flags & epAF_Zero)
    memset(pMemory, 0, size);
#endif
  return pMemory;
}

void *_epRealloc(void *pMemory, size_t size IF_MEMORY_DEBUG(const char * pFile, int line))
{
#if defined(EP_COMPILER_VISUALC)
  pMemory = _aligned_realloc(pMemory, size, EP_DEFAULT_ALIGNMENT);
#else
  pMemory = realloc(pMemory, size);
#endif // defined(_MSC_VER)

  return pMemory;
}

void _epFree(void *pMemory IF_MEMORY_DEBUG(const char * pFile, int line))
{
  if (pMemory)
  {
#if defined(EP_COMPILER_VISUALC)
    _aligned_free(pMemory);
#else
    free(pMemory);
#endif
  }
}

#if EP_DEBUG
# if !defined(EP_WINDOWS)
#   if defined(EP_COMPILER_GCC)
    #include <sys/stat.h>
    #include <fcntl.h>

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
