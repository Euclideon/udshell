#if !defined(_EP_PLUGIN_H)
#define _EP_PLUGIN_H

#include "ep/c/platform.h"
#include "ep/version.h"

#if EP_SHAREDLIB
# if defined(EP_COMPILER_VISUALC)
#   if defined(EP_ARCH_X86)
#     pragma comment (linker, "/export:_epPlugin_Init")
#   else
#     pragma comment (linker, "/export:epPlugin_Init")
#   endif
# endif
#endif

#if defined(__cplusplus)
extern "C" {
#endif

struct epKernel;

/**
* Get the active kernel instance.
*/
epKernel* epPlugin_GetKernel();

void* epPlugin_Alloc(size_t size);
void* epPlugin_AllocAligned(size_t size, size_t alignment);
void epPlugin_Free(void *pMem);

#if defined(__cplusplus)
}
#endif

#endif
