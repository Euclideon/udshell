#pragma once
#ifndef EPKERNEL_H
#define EPKERNEL_H

#include "components/component.h"
#include "components/lua.h"
#include "components/view.h"
#include "components/timer.h"
#include "components/logger.h"
#include "helpers.h"

#include "ep/epfastdelegate.h"
using namespace fastdelegate;

struct udRenderEngine;
struct epPluginInstance;

namespace ep
{

class LuaState;
class Renderer;

SHARED_CLASS(View);
SHARED_CLASS(UIComponent);
SHARED_CLASS(Window);
SHARED_CLASS(Logger);
SHARED_CLASS(PluginManager);
SHARED_CLASS(ResourceManager);
SHARED_CLASS(ShortcutManager);

class Kernel
{
public:
  // TODO: MessageHandler returns void, should we return some error state??
  typedef FastDelegate<void(String sender, String message, const Variant &data)> MessageHandler;

  static epResult Create(Kernel **ppInstance, InitParams commandLine, int renderThreadCount = 0);
  virtual epResult Destroy();

  epResult SendMessage(String target, String sender, String message, const Variant &data);

  void RegisterMessageHandler(SharedString name, MessageHandler messageHandler);

  // synchronisation
  typedef FastDelegate<void(Kernel*)> MainThreadCallback;
  void DispatchToMainThread(MainThreadCallback callback);
  void DispatchToMainThreadAndWait(MainThreadCallback callback);

  // component registry
  epResult RegisterComponentType(ComponentDesc *pDesc);
  template<typename ComponentType>
  epResult RegisterComponent();

  const ComponentDesc* GetComponentDesc(String id);

  template<typename CT>
  Array<const ComponentDesc *> GetDerivedComponentDescs(bool bIncludeBase)
  {
    return GetDerivedComponentDescs(&CT::descriptor, bIncludeBase);
  }
  Array<const ComponentDesc *> GetDerivedComponentDescs(const ComponentDesc *pBase, bool bIncludeBase);

  epResult CreateComponent(String typeId, InitParams initParams, ComponentRef *pNewInstance);

  template<typename T>
  SharedPtr<T> CreateComponent(InitParams initParams = nullptr);

  ComponentRef FindComponent(String uid);

  Renderer *GetRenderer() const { return pRenderer; }

  // script
  LuaRef GetLua() const { return spLua; }
  void Exec(String code);

  // logger functions
  LoggerRef GetLogger() const { return spLogger; }
  template<typename ...Args> void LogError(String text, Args... args) const;
  template<typename ...Args> void LogWarning(int level, String text, Args... args) const;
  template<typename ...Args> void LogDebug(int level, String text, Args... args) const;
  template<typename ...Args> void LogInfo(int level, String text, Args... args) const;
  template<typename ...Args> void LogScript(String text, Args... args) const;
  template<typename ...Args> void LogTrace(String text, Args... args) const;

  // Functions for resource management
  ResourceManagerRef GetResourceManager() const { return spResourceManager; }

  epResult RegisterExtensions(const ComponentDesc *pDesc, const Slice<const String> exts);
  DataSourceRef CreateDataSourceFromExtension(String ext, InitParams initParams);

  // other functions
  ViewRef GetFocusView() const { return spFocusView; }
  ViewRef SetFocusView(ViewRef spView);
  ShortcutManagerRef GetShortcutManager() const { return spShortcutManager; }

  virtual epResult RunMainLoop() { return epR_Success; }
  epResult Terminate();

protected:
  friend class Component;
  friend class PluginManager;

  struct ComponentType
  {
    const ComponentDesc *pDesc;
    size_t createCount;
  };
  struct MessageCallback
  {
    SharedString name;
    MessageHandler callback;
  };

  struct ForeignInstance
  {
    SharedString localUID;
    SharedString remoteUID;
    SharedString kernelUID;
  };

  SharedString uid;

  udHashMap<ComponentType> componentRegistry;
  udHashMap<Component*> instanceRegistry;
  udHashMap<ForeignInstance> foreignInstanceRegistry;
  udHashMap<MessageCallback> messageHandlers;

  AVLTree<String, const ComponentDesc *> extensionsRegistry;

  Renderer *pRenderer = nullptr;

  LuaRef spLua = nullptr;

  LoggerRef spLogger = nullptr;
  PluginManagerRef spPluginManager = nullptr;
  ResourceManagerRef spResourceManager = nullptr;
  ShortcutManagerRef spShortcutManager = nullptr;
  ViewRef spFocusView = nullptr;
  TimerRef spStreamerTimer = nullptr;
  TimerRef spUpdateTimer = nullptr;

  epPluginInstance *pPluginInstance = nullptr;

  virtual ~Kernel() {}

  epResult DoInit(Kernel *pKernel);

  static Kernel *CreateInstanceInternal(InitParams commandLine);
  virtual epResult InitInternal() = 0;

  epResult InitComponents();
  epResult InitRender();
  epResult DeinitRender();

  epResult DestroyComponent(Component *pInstance);

  epResult ReceiveMessage(String sender, String message, const Variant &data);

  int SendMessage(LuaState L);

  epPluginInstance *GetPluginInterface();

  void Update();
  void StreamerUpdate();

  template<typename CT>
  static Component *NewComponent(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, InitParams initParams);
};

template<typename T>
SharedPtr<T> Kernel::CreateComponent(InitParams initParams)
{
  ComponentRef c = nullptr;
  epResult r = CreateComponent(T::descriptor.id, initParams, &c);
  if (r != epR_Success)
    return nullptr;
  return shared_pointer_cast<T>(c);
}

template<typename CT>
Component *Kernel::NewComponent(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, InitParams initParams)
{
  MutableString128 t; t.format("New: {0} - {1}", pType->id, uid);
  pKernel->LogDebug(4, t);
  return udNew(CT, pType, pKernel, uid, initParams);
}
template<typename CT>
epResult Kernel::RegisterComponent()
{
  if (!CT::descriptor.pCreateInstance)
    CT::descriptor.pCreateInstance = &NewComponent<CT>;
  return RegisterComponentType(&CT::descriptor);
}


template<typename ...Args>
inline void Kernel::LogError(String text, Args... args) const
{
  if (sizeof...(Args) == 0)
    spLogger->Log(LogDefaults::LogLevel, text, LogCategories::Error, nullptr);
  else
  {
    MutableString128 tmp; tmp.format(text, args...);
    spLogger->Log(LogDefaults::LogLevel, tmp, LogCategories::Error, nullptr);
  }
}
template<typename ...Args>
inline void Kernel::LogWarning(int level, String text, Args... args) const
{
  if (sizeof...(Args) == 0)
    spLogger->Log(level, text, LogCategories::Warning, nullptr);
  else
  {
    MutableString128 tmp; tmp.format(text, args...);
    spLogger->Log(level, tmp, LogCategories::Warning, nullptr);
  }
}
template<typename ...Args>
inline void Kernel::LogDebug(int level, String text, Args... args) const
{
  if (sizeof...(Args) == 0)
    spLogger->Log(level, text, LogCategories::Debug, nullptr);
  else
  {
    MutableString128 tmp; tmp.format(text, args...);
    spLogger->Log(level, tmp, LogCategories::Debug, nullptr);
  }
}
template<typename ...Args>
inline void Kernel::LogInfo(int level, String text, Args... args) const
{
  if (sizeof...(Args) == 0)
    spLogger->Log(level, text, LogCategories::Info, nullptr);
  else
  {
    MutableString128 tmp; tmp.format(text, args...);
    spLogger->Log(level, tmp, LogCategories::Info, nullptr);
  }
}
template<typename ...Args>
inline void Kernel::LogScript(String text, Args... args) const
{
  if (sizeof...(Args) == 0)
    spLogger->Log(LogDefaults::LogLevel, text, LogCategories::Script, nullptr);
  else
  {
    MutableString128 tmp; tmp.format(text, args...);
    spLogger->Log(LogDefaults::LogLevel, tmp, LogCategories::Script, nullptr);
  }
}
template<typename ...Args>
inline void Kernel::LogTrace(String text, Args... args) const
{
  if (sizeof...(Args) == 0)
    spLogger->Log(LogDefaults::LogLevel, text, LogCategories::Trace, nullptr);
  else
  {
    MutableString128 tmp; tmp.format(text, args...);
    spLogger->Log(LogDefaults::LogLevel, tmp, LogCategories::Trace, nullptr);
  }
}

} // namespace ep

#endif // EPKERNEL_H
