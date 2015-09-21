#pragma once
#ifndef UDKERNEL_H
#define UDKERNEL_H

#include "components/component.h"
#include "components/lua.h"
#include "components/view.h"
#include "components/timer.h"
#include "helpers.h"

#include "3rdparty/FastDelegate.h"
using namespace fastdelegate;

struct udRenderEngine;

namespace ud
{

class LuaState;
class udBlockStreamer;
class Renderer;

SHARED_CLASS(View);
SHARED_CLASS(UIComponent);
SHARED_CLASS(Window);
SHARED_CLASS(Logger);
SHARED_CLASS(ResourceManager);

class Kernel
{
  friend class Component;
public:
  // TODO: MessageHandler returns void, should we return some error state??
  typedef FastDelegate<void(udString sender, udString message, const udVariant &data)> MessageHandler;

  static udResult Create(Kernel **ppInstance, udInitParams commandLine, int renderThreadCount = 0);
  udResult Destroy();

  udResult SendMessage(udString target, udString sender, udString message, const udVariant &data);

  void RegisterMessageHandler(udSharedString name, MessageHandler messageHandler);

  // synchronisation
  typedef FastDelegate<void(Kernel*)> MainThreadCallback;
  void DispatchToMainThread(MainThreadCallback callback);
  void DispatchToMainThreadAndWait(MainThreadCallback callback);

  // component registry
  udResult RegisterComponentType(ComponentDesc *pDesc);
  template<typename ComponentType>
  udResult RegisterComponent();

  template<typename CT>
  udFixedSlice<const ComponentDesc *> GetDerivedComponentDescs(bool bIncludeBase)
  {
    return GetDerivedComponentDescs(&CT::descriptor, bIncludeBase);
  }
  udFixedSlice<const ComponentDesc *> GetDerivedComponentDescs(const ComponentDesc *pBase, bool bIncludeBase);

  udResult CreateComponent(udString typeId, udInitParams initParams, ComponentRef *pNewInstance);

  template<typename T>
  udSharedPtr<T> CreateComponent(udInitParams initParams = nullptr);

  ComponentRef FindComponent(udString uid);

  Renderer *GetRenderer() const { return pRenderer; }

  // script
  LuaRef GetLua() const { return spLua; }
  void Exec(udString code);

  // other functions
  ViewRef GetFocusView() const { return spFocusView; }
  ViewRef SetFocusView(ViewRef spView);

  // logger functions
  LoggerRef GetLogger() const { return spLogger; }
  void LogError(const udString text, const udString componentUID = nullptr);
  void LogWarning(int level, const udString text, const udString componentUID = nullptr);
  void LogDebug(int level, const udString text, const udString componentUID = nullptr);
  void LogInfo(int level, const udString text, const udString componentUID = nullptr);
  void LogScript(const udString text, const udString componentUID = nullptr);
  void LogTrace(const udString text, const udString componentUID = nullptr);
  void Log(const udString text, const udString componentUID = nullptr);

  // Functions for resource management
  ResourceManagerRef GetResourceManager() const { return spResourceManager; }

  udResult RegisterExtensions(const ComponentDesc *pDesc, const udSlice<const udString> exts);
  DataSourceRef CreateDataSourceFromExtension(udString ext, udInitParams initParams);

  udResult RunMainLoop();
  udResult Terminate();

protected:
  struct ComponentType
  {
    const ComponentDesc *pDesc;
    size_t createCount;
  };
  struct MessageCallback
  {
    udSharedString name;
    MessageHandler callback;
  };

  struct ForeignInstance
  {
    udSharedString localUID;
    udSharedString remoteUID;
    udSharedString kernelUID;
  };

  udSharedString uid;

  udHashMap<ComponentType> componentRegistry;
  udHashMap<Component*> instanceRegistry;
  udHashMap<ForeignInstance> foreignInstanceRegistry;
  udHashMap<MessageCallback> messageHandlers;

  Renderer *pRenderer = nullptr;

  LuaRef spLua = nullptr;

  LoggerRef spLogger = nullptr;
  ResourceManagerRef spResourceManager = nullptr;
  ViewRef spFocusView = nullptr;
  TimerRef spStreamerTimer = nullptr;
  TimerRef spUpdateTimer = nullptr;

  udResult DoInit(Kernel *pKernel);

  static Kernel *CreateInstanceInternal(udInitParams commandLine);
  udResult InitInstanceInternal();
  udResult DestroyInstanceInternal();

  udResult InitComponents();
  udResult InitRender();
  udResult DeinitRender();

  udResult DestroyComponent(Component *pInstance);

  udResult ReceiveMessage(udString sender, udString message, const udVariant &data);

  int SendMessage(LuaState L);

  void Update();
  void StreamerUpdate();

  template<typename CT>
  static Component *NewComponent(const ComponentDesc *pType, Kernel *pKernel, udSharedString uid, udInitParams initParams);

  udAVLTree<udString, const ComponentDesc *> extensionsRegistry;
};

template<typename T>
udSharedPtr<T> Kernel::CreateComponent(udInitParams initParams)
{
  ComponentRef c = nullptr;
  udResult r = CreateComponent(T::descriptor.id, initParams, &c);
  if (r != udR_Success)
    return nullptr;
  return shared_pointer_cast<T>(c);
}

template<typename CT>
Component *Kernel::NewComponent(const ComponentDesc *pType, Kernel *pKernel, udSharedString uid, udInitParams initParams)
{
  udMutableString128 t; t.format("New: {0} - {1}", pType->id, uid);
  pKernel->LogDebug(4, t);
  return udNew(CT, pType, pKernel, uid, initParams);
}
template<typename CT>
udResult Kernel::RegisterComponent()
{
  if (!CT::descriptor.pCreateInstance)
    CT::descriptor.pCreateInstance = &NewComponent<CT>;
  return RegisterComponentType(&CT::descriptor);
}

} //namespace ud

#endif // UDKERNEL_H
