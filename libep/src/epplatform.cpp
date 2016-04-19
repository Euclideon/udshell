#include "ep/cpp/platform.h"

#include <stdio.h>
#include "ep/cpp/plugin.h"

namespace ep {
namespace internal {

bool gUnitTesting = false;

MutableString256 assertBuffer;

void *_Alloc(size_t size, epAllocationFlags flags, const char * pFile, int line)
{
#if defined(EP_COMPILER_VISUALC)
# if __EP_MEMORY_DEBUG__
  void *pMemory = (flags & epAF_Zero) ? _recalloc_dbg(nullptr, 1, size, _NORMAL_BLOCK, pFile, line) : _malloc_dbg(size, _NORMAL_BLOCK, pFile, line);
# else
  void *pMemory = (flags & epAF_Zero) ? _recalloc(nullptr, 1, size) : malloc(size);
# endif // __EP_MEMORY_DEBUG__
#else // defined(EP_COMPILER_VISUALC)
  epUnused(pFile);
  epUnused(line);
  void *pMemory = (flags & epAF_Zero) ? calloc(1, size) : malloc(size);
#endif
  return pMemory;
}

void *_Realloc(void *pMemory, size_t size, const char * pFile, int line)
{
#if defined(EP_COMPILER_VISUALC)
# if __EP_MEMORY_DEBUG__
  pMemory = _realloc_dbg(pMemory, size, _NORMAL_BLOCK, pFile, line);
# else
  pMemory = realloc(pMemory, size);
# endif // __EP_MEMORY_DEBUG__
#else
  epUnused(pFile);
  epUnused(line);
  pMemory = realloc(pMemory, size);
#endif // defined(_MSC_VER)

  return pMemory;
}

void _Free(void *pMemory)
{
  if (pMemory)
    free(pMemory);
}

} // namespace internal
} // namespace ep

// Plugin API API
extern "C" {

void epAssertFailed(epString condition, epString message, epString file, int line)
{
  // TODO: Agree on formatting of assets
  if (ep::internal::gUnitTesting)
  {
    fprintf(stderr, "%.*s", (int)message.length, message.ptr);
    exit(EXIT_FAILURE);
  }
  else
  {
    ep::MutableString256 t(ep::Format, "ASSERT FAILED : {0}\n{1}\n{2}({3})", (ep::String&)condition, (ep::String&)message, (ep::String&)file, line);
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

  size_t len = ep::internal::epvscprintf(format, args) + 1;
  char *pBuffer = (char*)alloca(len + 1);
  len = ep::internal::epvsnprintf(pBuffer, len, format, args);
  va_end(args);

  epDebugWrite(pBuffer);
}

void *_epAlloc(size_t size, epAllocationFlags flags EP_IF_MEMORY_DEBUG(const char * pFile, int line))
{
#if __EP_MEMORY_DEBUG__
  if (ep::s_pInstance)
    return ep::s_pInstance->Alloc(size, flags, pFile, line);

  return ep::internal::_Alloc(size, flags, pFile, line);
#else
  if (ep::s_pInstance)
    return ep::s_pInstance->Alloc(size, flags, nullptr, 0);

  return ep::internal::_Alloc(size, flags, nullptr, 0);
#endif // __EP_MEMORY_DEBUG__
}

void _epFree(void *pMemory)
{
  if (ep::s_pInstance)
  {
    ep::s_pInstance->Free(pMemory);
    return;
  }

  ep::internal::_Free(pMemory);
}

#if __EP_MEMORY_DEBUG__
namespace ep {
namespace internal {

int reportingHook(int reportType, char* userMessage, int* retVal)
{
  static bool filter = true;
  static int debugMsgCount = 3;
  static int leakCount = 0;

  if (strcmp(userMessage, "Object dump complete.\n") == 0)
    filter = false;

  if (filter)
  {
    // Debug messages from our program should consist of 4 parts :
    // File (line) | AllocID | Block Descriptor | Memory Data
    if (!strstr(userMessage, ") : "))
    {
      ++debugMsgCount;
    }
    else
    {
      if (leakCount == 0)
        OutputDebugStringA("Detected memory leaks!\nDumping objects ->\n");
      debugMsgCount = 0;
      ++leakCount;
    }
    // Filter the output if it's not from our program
    return (debugMsgCount > 3);
  }

  return (leakCount == 0);
}

} // namespace internal
} // namespace ep

void epInitMemoryTracking()
{
#if defined(EP_WINDOWS)
  const char *pFilename = "MemoryReport_"
#if EP_DEBUG
    "Debug_"
#else
    "Release_"
#endif // EP_DEBUG
#if defined(EP_ARCH_X64)
    "x64"
#elif defined(EP_ARCH_X86)
    "x86"
#else
#   error "Couldn't detect target architecture"
#endif // defined (EP_ARCH_X64)
    ".txt";

  HANDLE hCrtWarnReport = CreateFile(pFilename, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
  if (hCrtWarnReport == INVALID_HANDLE_VALUE) OutputDebugStringA("Error creating CrtWarnReport.txt");

  errno = 0;
  int warnMode = _CrtSetReportMode(_CRT_WARN, _CRTDBG_REPORT_MODE);
  _CrtSetReportMode(_CRT_WARN, warnMode | _CRTDBG_MODE_FILE);
  if (errno == EINVAL) OutputDebugStringA("Error calling _CrtSetReportMode() warnings");

  errno = 0;
  _CrtSetReportFile(_CRT_WARN, hCrtWarnReport);
  if (errno == EINVAL)OutputDebugStringA("Error calling _CrtSetReportFile() warnings");
  _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

  //change the report function to only report memory leaks from program code
  _CrtSetReportHook(ep::internal::reportingHook);
#endif
}
#endif // __EP_MEMORY_DEBUG__

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
