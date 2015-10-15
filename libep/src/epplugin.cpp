#include "ep/c/plugin.h"

extern "C" {

epPluginInstance *s_pPluginInstance = nullptr;

bool epPluginAttach();

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
#endif

EP_EXPORT bool epPlugin_Init(epPluginInstance *pPlugin)
{
  s_pPluginInstance = pPlugin;
  return epPluginAttach();
}


epKernel *epPlugin_GetKernel()
{
  return s_pPluginInstance->pKernelInstance;
}

void* epPlugin_Alloc(size_t size)
{
  return s_pPluginInstance->Alloc(size);
}

void* epPlugin_AllocAligned(size_t size, size_t alignment)
{
  return s_pPluginInstance->AllocAligned(size, alignment);
}

void epPlugin_Free(void *pMem)
{
  s_pPluginInstance->Free(pMem);
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
