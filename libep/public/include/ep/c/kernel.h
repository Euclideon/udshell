#if !defined(_EP_KERNEL_H)
#define _EP_KERNEL_H

#include "ep/c/component.h"

#if defined(__cplusplus)
extern "C" {
#endif

struct epKernel;

struct epKernelAPI
{
//  epResult (*Destroy)();
  epResult (*pSendMessage)(epKernel *pKernel, epString target, epString sender, epString message, const epVariant* pData);
//  void (*RegisterMessageHandler)(epKernel *pKernel, SharedString name, MessageHandler messageHandler);

  // synchronisation
//  typedef FastDelegate<void(Kernel*)> MainThreadCallback;
//  void DispatchToMainThread(epKernel *pKernel, MainThreadCallback callback);
//  void DispatchToMainThreadAndWait(epKernel *pKernel, MainThreadCallback callback);

  // component registry
  epResult (*pRegisterComponentType)(epKernel *pKernel, const epComponentDesc *pDesc);

//  epFixedSlice<const epComponentDesc*> GetDerivedComponentDescs(epKernel *pKernel, const epComponentDesc *pBase, bool bIncludeBase);

  epResult (*pCreateComponent)(epKernel *pKernel, epString typeId, const epKeyValuePair *pInitParams, size_t numInitParams, epComponent **ppNewInstance);

  epComponent* (*pFindComponent)(epKernel *pKernel, epString uid);

  // script
//  LuaRef GetLua() const { return spLua; }
  void (*pExec)(epKernel *pKernel, epString code);
/*
  // other functions
  epComponent* GetFocusView(epKernel *pKernel) const { return spFocusView; }
  ViewRef SetFocusView(epKernel *pKernel, epComponent *pView);

  // logger functions
  LoggerRef GetLogger(epKernel *pKernel) const { return spLogger; }
*/
  void (*pLogError)(epKernel *pKernel, const epString text, const epString componentUID);
  void (*pLogWarning)(epKernel *pKernel, int level, const epString text, const epString componentUID);
  void (*pLogDebug)(epKernel *pKernel, int level, const epString text, const epString componentUID);
  void (*pLogInfo)(epKernel *pKernel, int level, const epString text, const epString componentUID);
  void (*pLogScript)(epKernel *pKernel, const epString text, const epString componentUID);
  void (*pLogTrace)(epKernel *pKernel, const epString text, const epString componentUID);
/*
  void (*Log)(epKernel *pKernel, const epString text, const epString componentUID = nullptr);

  // Functions for resource management
  ResourceManagerRef GetResourceManager(epKernel *pKernel) const { return spResourceManager; }

  epResult RegisterExtensions(epKernel *pKernel, const ComponentDesc *pDesc, const Slice<const epString> exts);
  DataSourceRef CreateDataSourceFromExtension(epKernel *pKernel, epString ext, InitParams initParams);

  epResult RunMainLoop(epKernel *pKernel);
  epResult Terminate(epKernel *pKernel);
*/
};

static inline epResult     epKernel_SendMessage(epString target, epString sender, epString message, const epVariant* pData) { return s_pPluginInstance->pKernelAPI->pSendMessage(s_pPluginInstance->pKernelInstance, target, sender, message, pData); }
static inline epResult     epKernel_RegisterComponentType(const epComponentDesc *pDesc)                                     { return s_pPluginInstance->pKernelAPI->pRegisterComponentType(s_pPluginInstance->pKernelInstance, pDesc); }
static inline epResult     epKernel_CreateComponent(epString typeId, const epKeyValuePair *pInitParams, size_t numInitParams, epComponent **ppNewInstance) { return s_pPluginInstance->pKernelAPI->pCreateComponent(s_pPluginInstance->pKernelInstance, typeId, pInitParams, numInitParams, ppNewInstance); }
static inline epComponent* epKernel_FindComponent(epString uid)                                                             { return s_pPluginInstance->pKernelAPI->pFindComponent(s_pPluginInstance->pKernelInstance, uid); }
static inline void         epKernel_Exec(epString code)                                                                     { s_pPluginInstance->pKernelAPI->pExec(s_pPluginInstance->pKernelInstance, code); }
static inline void         epKernel_LogError(const epString text, const epString componentUID)                              { s_pPluginInstance->pKernelAPI->pLogError(s_pPluginInstance->pKernelInstance, text, componentUID); }
static inline void         epKernel_LogWarning(int level, const epString text, const epString componentUID)                 { s_pPluginInstance->pKernelAPI->pLogWarning(s_pPluginInstance->pKernelInstance, level, text, componentUID); }
static inline void         epKernel_LogDebug(int level, const epString text, const epString componentUID)                   { s_pPluginInstance->pKernelAPI->pLogDebug(s_pPluginInstance->pKernelInstance, level, text, componentUID); }
static inline void         epKernel_LogInfo(int level, const epString text, const epString componentUID)                    { s_pPluginInstance->pKernelAPI->pLogInfo(s_pPluginInstance->pKernelInstance, level, text, componentUID); }
static inline void         epKernel_LogScript(const epString text, const epString componentUID)                             { s_pPluginInstance->pKernelAPI->pLogScript(s_pPluginInstance->pKernelInstance, text, componentUID); }
static inline void         epKernel_LogTrace(const epString text, const epString componentUID)                              { s_pPluginInstance->pKernelAPI->pLogTrace(s_pPluginInstance->pKernelInstance, text, componentUID); }

#if defined(__cplusplus)
}
#endif

#endif
