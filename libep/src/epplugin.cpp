#include "ep/epplugin.h"

extern "C" {

epPluginInstance *s_pPluginInstance = nullptr;

bool epPlugin_Init(epPluginInstance *pPlugin)
{
  s_pPluginInstance = pPlugin;

  // TODO: register exports...

  return false;
}

}

#if defined(EP_WINDOWS)
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
