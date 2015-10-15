#if !defined(_EP_KERNEL_H)
#define _EP_KERNEL_H

#include "ep/c/component.h"

#if defined(__cplusplus)
extern "C" {
#endif

struct epKernel;

epResult     epKernel_SendMessage(epString target, epString sender, epString message, const epVariant* pData);
epResult     epKernel_RegisterComponentType(const epComponentDesc *pDesc);
epResult     epKernel_CreateComponent(epString typeId, const epKeyValuePair *pInitParams, size_t numInitParams, epComponent **ppNewInstance);
epComponent* epKernel_FindComponent(epString uid);
void         epKernel_Exec(epString code);
void         epKernel_LogError(const epString text, const epString componentUID);
void         epKernel_LogWarning(int level, const epString text, const epString componentUID);
void         epKernel_LogDebug(int level, const epString text, const epString componentUID);
void         epKernel_LogInfo(int level, const epString text, const epString componentUID);
void         epKernel_LogScript(const epString text, const epString componentUID);
void         epKernel_LogTrace(const epString text, const epString componentUID);


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

#if defined(__cplusplus)
}
#endif

#endif
