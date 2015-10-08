#pragma once
#if !defined(_EP_PLUGIN_H)
#define _EP_PLUGIN_H

#include "ep/epversion.h"
#include "ep/epcomponent.h"

typedef unsigned int epResult;

extern "C" {

struct epKernel;

struct epKernelAPI
{
//  epResult (*Destroy)();
  epResult (*SendMessage)(epKernel *pKernel, epString target, epString sender, epString message, const epVariant* pData);
//  void (*RegisterMessageHandler)(epKernel *pKernel, epSharedString name, MessageHandler messageHandler);

  // synchronisation
//  typedef FastDelegate<void(Kernel*)> MainThreadCallback;
//  void DispatchToMainThread(epKernel *pKernel, MainThreadCallback callback);
//  void DispatchToMainThreadAndWait(epKernel *pKernel, MainThreadCallback callback);

  // component registry
  epResult (*RegisterComponentType)(epKernel *pKernel, const epComponentDesc *pDesc);

//  epFixedSlice<const epComponentDesc*> GetDerivedComponentDescs(epKernel *pKernel, const epComponentDesc *pBase, bool bIncludeBase);

  epResult (*CreateComponent)(epKernel *pKernel, epString typeId, epInitParams initParams, epComponent **ppNewInstance);

  epComponent* (*FindComponent)(epKernel *pKernel, epString uid);

  // script
//  LuaRef GetLua() const { return spLua; }
  void (*Exec)(epKernel *pKernel, epString code);
/*
  // other functions
  epComponent* GetFocusView(epKernel *pKernel) const { return spFocusView; }
  ViewRef SetFocusView(epKernel *pKernel, epComponent *pView);

  // logger functions
  LoggerRef GetLogger(epKernel *pKernel) const { return spLogger; }
*/
  void (*LogError)(epKernel *pKernel, const epString text, const epString componentUID);
  void (*LogWarning)(epKernel *pKernel, int level, const epString text, const epString componentUID);
  void (*LogDebug)(epKernel *pKernel, int level, const epString text, const epString componentUID);
  void (*LogInfo)(epKernel *pKernel, int level, const epString text, const epString componentUID);
  void (*LogScript)(epKernel *pKernel, const epString text, const epString componentUID);
  void (*LogTrace)(epKernel *pKernel, const epString text, const epString componentUID);
/*
  void (*Log)(epKernel *pKernel, const epString text, const epString componentUID = nullptr);

  // Functions for resource management
  ResourceManagerRef GetResourceManager(epKernel *pKernel) const { return spResourceManager; }

  epResult RegisterExtensions(epKernel *pKernel, const ComponentDesc *pDesc, const epSlice<const epString> exts);
  DataSourceRef CreateDataSourceFromExtension(epKernel *pKernel, epString ext, epInitParams initParams);

  epResult RunMainLoop(epKernel *pKernel);
  epResult Terminate(epKernel *pKernel);
*/
};

struct epPluginInstance
{
  int apiVersion;

  epKernel *pKernelInstance;

  void(*DestroyComponent)(epComponent *pInstance);

  epKernelAPI *pKernelAPI;
  epComponentAPI *pComponentAPI;
};


#if defined(EP_PLUGIN)
extern static epPluginInstance *s_pPluginInstance;
#endif

}

#endif
