#include "ep/cpp/plugin.h"

namespace ep {

Instance *s_pInstance = nullptr;

} // namespace ep

extern "C" {

bool epPluginAttach() epweak;

#if defined(EP_COMPILER_VISUALC)
bool epPluginAttachWeak()
{
  return false;
}
# if defined(EP_ARCH_X86)
#   pragma comment(linker, "/alternatename:_epPluginAttach=_epPluginAttachWeak")
# else
#   pragma comment(linker, "/alternatename:epPluginAttach=epPluginAttachWeak")
# endif
#else
bool epPluginAttach()
{
  return false;
}
#endif

EP_EXPORT bool epPlugin_Init(ep::Instance *pPlugin)
{
  ep::s_pInstance = pPlugin;
  return epPluginAttach();
}

epKernel *epPlugin_GetKernel()
{
  return (epKernel*)ep::s_pInstance->pKernelInstance;
}

void* epPlugin_Alloc(size_t size)
{
  return ep::s_pInstance->Alloc(size);
}

void* epPlugin_AllocAligned(size_t size, size_t alignment)
{
  return ep::s_pInstance->AllocAligned(size, alignment);
}

void epPlugin_Free(void *pMem)
{
  ep::s_pInstance->Free(pMem);
}

}

#if 0 //defined(EP_WINDOWS) // TODO: why does the CRT hog this? we might need to do things here...
BOOLEAN WINAPI DllMain(IN HINSTANCE epUnusedParam(hDllHandle), IN DWORD nReason, IN LPVOID epUnusedParam(Reserved))
{
  BOOLEAN bSuccess = TRUE;

  // Perform global initialization.
  switch (nReason)
  {
    case DLL_PROCESS_ATTACH:
      // For optimization.
//      DisableThreadLibraryCalls(hDllHandle);
      break;

    case DLL_PROCESS_DETACH:
      break;

    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
      break;
  }

  return bSuccess;
}
#endif
