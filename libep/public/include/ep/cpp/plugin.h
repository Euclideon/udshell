#if !defined(_EP_PLUGIN_HPP)
#define _EP_PLUGIN_HPP

#include "ep/cpp/platform.h"
#include "ep/c/plugin.h"
#include "ep/cpp/hashmap.h"
#include "ep/cpp/sharedptr.h"

namespace ep {

class Kernel;
class Component;

struct Instance
{
  int apiVersion;

  Kernel *pKernelInstance;
  void *pErrorSystem;
  void *pStaticImplRegistry;
  void *pTreeAllocator;
  void *pWeakRegistry;

  void*(*Alloc)(size_t size, epAllocationFlags flags, const char *pFile, int line);
  void(*Free)(void *pMemory);

  void(*AssertFailed)(String condition, String message, String file, int line);

  void(*DestroyComponent)(Component *pInstance);

  void*(*Find)(String pattern, void *pHandle, void *pData);
};

extern Instance *s_pInstance;

} // namespace ep

#endif // _EP_PLUGIN_HPP
