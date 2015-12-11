#include "ep/cpp/platform.h"

#include "udPlatform.h"

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

} // extern "C"
