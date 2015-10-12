#pragma once
#if !defined(_EP_PLUGIN_H)
#define _EP_PLUGIN_H

#include "ep/epplatform.h"
#include "ep/epversion.h"

#if EP_SHAREDLIB
# if defined(EP_COMPILER_VISUALC)
#   pragma comment (linker, "/export:epPlugin_Init")
# endif
#endif

extern "C" {

struct epKernel;
struct epComponent;

struct epPluginInstance
{
  int apiVersion;

  epKernel *pKernelInstance;

  void(*DestroyComponent)(epComponent *pInstance);

  struct epKernelAPI *pKernelAPI;
  struct epComponentAPI *pComponentAPI;
};

extern epPluginInstance *s_pPluginInstance;

/**
 * Get the active kernel instance.
 */
inline epKernel *epPlugin_GetKernel() { return s_pPluginInstance->pKernelInstance; }

}

#endif
