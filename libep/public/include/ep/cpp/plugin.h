#if !defined(_EP_PLUGIN_HPP)
#define _EP_PLUGIN_HPP

#include "ep/cpp/platform.h"
#include "ep/c/plugin.h"

namespace ep {

class Kernel;
class Component;

struct Instance
{
  int apiVersion;

  Kernel *pKernelInstance;

  void*(*Alloc)(size_t size, epAllocationFlags flags, const char *pFile, int line);
  void(*Free)(void *pMemory);

  void(*AssertFailed)(String condition, String message, String file, int line);

  void(*DestroyComponent)(Component *pInstance);
  void*(*TreeAllocator)();

  void*(*WeakRegistry)();

  void*(*Find)(String pattern, void *pHandle, void *pData);
};

extern Instance *s_pInstance;

} // namespace ep

#endif // _EP_PLUGIN_HPP
