#pragma once
#ifndef EPKERNEL_H
#define EPKERNEL_H

#include "components/component.h"
#include "components/lua.h"
#include "components/view.h"
#include "components/timer.h"
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
  typedef FastDelegate<void(epString sender, epString message, const epVariant &data)> MessageHandler;

  static epResult Create(Kernel **ppInstance, epInitParams commandLine, int renderThreadCount = 0);
  virtual epResult Destroy();

  epResult SendMessage(epString target, epString sender, epString message, const epVariant &data);

  void RegisterMessageHandler(epSharedString name, MessageHandler messageHandler);

  // synchronisation
  typedef FastDelegate<void(Kernel*)> MainThreadCallback;
  void DispatchToMainThread(MainThreadCallback callback);
  void DispatchToMainThreadAndWait(MainThreadCallback callback);

  // component registry
  epResult RegisterComponentType(ComponentDesc *pDesc);
  template<typename ComponentType>
  epResult RegisterComponent();

  const ComponentDesc* GetComponentDesc(epString id);

  template<typename CT>
  epArray<const ComponentDesc *> GetDerivedComponentDescs(bool bIncludeBase)
  {
    return GetDerivedComponentDescs(&CT::descriptor, bIncludeBase);
  }
  epArray<const ComponentDesc *> GetDerivedComponentDescs(const ComponentDesc *pBase, bool bIncludeBase);

  epResult CreateComponent(epString typeId, epInitParams initParams, ComponentRef *pNewInstance);

  template<typename T>
  epSharedPtr<T> CreateComponent(epInitParams initParams = nullptr);

  ComponentRef FindComponent(epString uid);

  Renderer *GetRenderer() const { return pRenderer; }

  // script
  LuaRef GetLua() const { return spLua; }
  void Exec(epString code);

  // logger functions
  LoggerRef GetLogger() const { return spLogger; }
  template<typename ...Args> void LogError(epString text, Args... args) const;
  template<typename ...Args> void LogWarning(int level, epString text, Args... args) const;
  template<typename ...Args> void LogDebug(int level, epString text, Args... args) const;
  template<typename ...Args> void LogInfo(int level, epString text, Args... args) const;
  template<typename ...Args> void LogScript(epString text, Args... args) const;
  template<typename ...Args> void LogTrace(epString text, Args... args) const;
  template<typename ...Args> void Log(epString text, Args... args) const; // Calls LogDebug() with level 2

  // Functions for resource management
  ResourceManagerRef GetResourceManager() const { return spResourceManager; }

  epResult RegisterExtensions(const ComponentDesc *pDesc, const epSlice<const epString> exts);
  DataSourceRef CreateDataSourceFromExtension(epString ext, epInitParams initParams);

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
    epSharedString name;
    MessageHandler callback;
  };

  struct ForeignInstance
  {
    epSharedString localUID;
    epSharedString remoteUID;
    epSharedString kernelUID;
  };

  epSharedString uid;

  udHashMap<ComponentType> componentRegistry;
  udHashMap<Component*> instanceRegistry;
  udHashMap<ForeignInstance> foreignInstanceRegistry;
  udHashMap<MessageCallback> messageHandlers;

  epAVLTree<epString, const ComponentDesc *> extensionsRegistry;

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

  static Kernel *CreateInstanceInternal(epInitParams commandLine);
  virtual epResult InitInternal() = 0;

  epResult InitComponents();
  epResult InitRender();
  epResult DeinitRender();

  epResult DestroyComponent(Component *pInstance);

  epResult ReceiveMessage(epString sender, epString message, const epVariant &data);

  int SendMessage(LuaState L);

  epPluginInstance *GetPluginInterface();

  void Update();
  void StreamerUpdate();

  template<typename CT>
  static Component *NewComponent(const ComponentDesc *pType, Kernel *pKernel, epSharedString uid, epInitParams initParams);
};

template<typename T>
epSharedPtr<T> Kernel::CreateComponent(epInitParams initParams)
{
  ComponentRef c = nullptr;
  epResult r = CreateComponent(T::descriptor.id, initParams, &c);
  if (r != epR_Success)
    return nullptr;
  return shared_pointer_cast<T>(c);
}

template<typename CT>
Component *Kernel::NewComponent(const ComponentDesc *pType, Kernel *pKernel, epSharedString uid, epInitParams initParams)
{
  epMutableString128 t; t.format("New: {0} - {1}", pType->id, uid);
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

} //namespace ep

#include "components/logger.h"
namespace ep
{

template<typename ...Args>
inline void Kernel::LogError(epString text, Args... args) const
{
  if (sizeof...(Args) == 0)
    spLogger->Log(LogDefaults::LogLevel, text, LogCategories::Error, nullptr);
  else
  {
    epMutableString128 tmp; tmp.format(text, args...);
    spLogger->Log(LogDefaults::LogLevel, tmp, LogCategories::Error, nullptr);
  }
}
template<typename ...Args>
inline void Kernel::LogWarning(int level, epString text, Args... args) const
{
  if (sizeof...(Args) == 0)
    spLogger->Log(level, text, LogCategories::Warning, nullptr);
  else
  {
    epMutableString128 tmp; tmp.format(text, args...);
    spLogger->Log(level, tmp, LogCategories::Warning, nullptr);
  }
}
template<typename ...Args>
inline void Kernel::LogDebug(int level, epString text, Args... args) const
{
  if (sizeof...(Args) == 0)
    spLogger->Log(level, text, LogCategories::Debug, nullptr);
  else
  {
    epMutableString128 tmp; tmp.format(text, args...);
    spLogger->Log(level, tmp, LogCategories::Debug, nullptr);
  }
}
template<typename ...Args>
inline void Kernel::LogInfo(int level, epString text, Args... args) const
{
  if (sizeof...(Args) == 0)
    spLogger->Log(level, text, LogCategories::Info, nullptr);
  else
  {
    epMutableString128 tmp; tmp.format(text, args...);
    spLogger->Log(level, tmp, LogCategories::Info, nullptr);
  }
}
template<typename ...Args>
inline void Kernel::LogScript(epString text, Args... args) const
{
  if (sizeof...(Args) == 0)
    spLogger->Log(LogDefaults::LogLevel, text, LogCategories::Script, nullptr);
  else
  {
    epMutableString128 tmp; tmp.format(text, args...);
    spLogger->Log(LogDefaults::LogLevel, tmp, LogCategories::Script, nullptr);
  }
}
template<typename ...Args>
inline void Kernel::LogTrace(epString text, Args... args) const
{
  if (sizeof...(Args) == 0)
    spLogger->Log(LogDefaults::LogLevel, text, LogCategories::Trace, nullptr);
  else
  {
    epMutableString128 tmp; tmp.format(text, args...);
    spLogger->Log(LogDefaults::LogLevel, tmp, LogCategories::Trace, nullptr);
  }
}
template<typename ...Args>
inline void Kernel::Log(epString text, Args... args) const
{
  if (sizeof...(Args) == 0)
    spLogger->Log(LogDefaults::LogLevel, text, LogCategories::Debug, nullptr);
  else
  {
    epMutableString128 tmp; tmp.format(text, args...);
    spLogger->Log(LogDefaults::LogLevel, tmp, LogCategories::Debug, nullptr);
  }
}

} // namespace ep

#endif // EPKERNEL_H
