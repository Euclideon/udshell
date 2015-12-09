#pragma once
#ifndef EPKERNEL_H
#define EPKERNEL_H

#include "components/componentimpl.h"
#include "components/lua.h"
#include "components/view.h"
#include "components/timer.h"
#include "components/logger.h"
#include "helpers.h"

#include "ep/cpp/hashmap.h"
#include "ep/cpp/kernel.h"

#include "ep/epfastdelegate.h"
using namespace fastdelegate;

struct udRenderEngine;
struct epPluginInstance;

namespace ep {

class LuaState;
class Renderer;
class StdCapture;

SHARED_CLASS(UIComponent);
SHARED_CLASS(Window);
SHARED_CLASS(Logger);
SHARED_CLASS(PluginManager);
SHARED_CLASS(NativePluginLoader);

}

namespace kernel {

class Kernel : public ep::Kernel
{
public:
  static epResult Create(Kernel **ppInstance, Slice<const KeyValuePair> commandLine, int renderThreadCount = 0);
  virtual epResult Destroy();

  epResult SendMessage(String target, String sender, String message, const Variant &data) override final;

  void RegisterMessageHandler(SharedString name, MessageHandler messageHandler) override final;

  // synchronisation
  void DispatchToMainThread(MainThreadCallback callback) override final;
  void DispatchToMainThreadAndWait(MainThreadCallback callback) override final;

  // component registry
  const ep::ComponentDesc* RegisterComponentType(const ep::ComponentDesc &desc) override final;
  template <typename ComponentType, typename Impl = void>
  inline const ep::ComponentDesc* RegisterComponentType() { return ep::Kernel::RegisterComponentType<ComponentType, Impl>(); }

  const ep::ComponentDesc* GetComponentDesc(String id) override final;

  template<typename CT>
  Array<const ComponentDesc *> GetDerivedComponentDescs(bool bIncludeBase)
  {
    return GetDerivedComponentDescs(&CT::descriptor, bIncludeBase);
  }
  Array<const ep::ComponentDesc *> GetDerivedComponentDescs(const ep::ComponentDesc *pBase, bool bIncludeBase);

  epResult CreateComponent(String typeId, Variant::VarMap initParams, ep::ComponentRef *pNewInstance) override final;
  template<typename T>
  SharedPtr<T> CreateComponent(Variant::VarMap initParams = nullptr) { return ep::Kernel::CreateComponent<T>(initParams); }

  ep::ComponentRef FindComponent(String uid) const override final;

  Renderer *GetRenderer() const { return pRenderer; }

  // script
  LuaRef GetLua() const { return spLua; }
  void Exec(String code) override;

  // logger functions
  LoggerRef GetLogger() const { return spLogger; }
  void Log(int kind, int level, String text, String component = nullptr) const override final { spLogger->Log(level, text, (LogCategories)kind, component); }
  template<typename ...Args> void LogError(String format, Args... args) const;
  template<typename ...Args> void LogWarning(int level, String format, Args... args) const;
  template<typename ...Args> void LogDebug(int level, String format, Args... args) const;
  template<typename ...Args> void LogInfo(int level, String format, Args... args) const;
  template<typename ...Args> void LogScript(String format, Args... args) const;
  template<typename ...Args> void LogTrace(String format, Args... args) const;

  // Functions for resource management
  ResourceManagerRef GetResourceManager() const override final { return spResourceManager; }

  epResult RegisterExtensions(const ep::ComponentDesc *pDesc, const Slice<const String> exts) override final;
  DataSourceRef CreateDataSourceFromExtension(String ext, Variant::VarMap initParams) override final;

  // stdio relaying functions
  BroadcasterRef GetStdOutBroadcaster() const override final { return spStdOutBC; }
  BroadcasterRef GetStdErrBroadcaster() const override final { return spStdErrBC; }

  // other functions
  ViewRef GetFocusView() const override final { return spFocusView; }
  ViewRef SetFocusView(ViewRef spView) override final;

  CommandManagerRef GetCommandManager() const override final { return spCommandManager; }

  virtual epResult RunMainLoop() { return epR_Success; }
  epResult Terminate();

  epPluginInstance *GetPluginInterface();

protected:
  friend class Component;
  friend class ep::ComponentImpl;

  Kernel();

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

  HashMap<ComponentType> componentRegistry;
  HashMap<Component*> instanceRegistry;
  HashMap<ForeignInstance> foreignInstanceRegistry;
  HashMap<MessageCallback> messageHandlers;

  AVLTree<String, const ep::ComponentDesc *> extensionsRegistry;

  Renderer *pRenderer = nullptr;

  LuaRef spLua = nullptr;

  LoggerRef spLogger = nullptr;
  PluginManagerRef spPluginManager = nullptr;
  ResourceManagerRef spResourceManager = nullptr;
  CommandManagerRef spCommandManager = nullptr;
  ViewRef spFocusView = nullptr;
  TimerRef spStreamerTimer = nullptr;
  TimerRef spUpdateTimer = nullptr;
  BroadcasterRef spStdOutBC = nullptr;
  BroadcasterRef spStdErrBC = nullptr;

  StdCapture *stdOutCapture = nullptr;
  StdCapture *stdErrCapture = nullptr;

  epPluginInstance *pPluginInstance = nullptr;

  virtual ~Kernel() {}

  epResult DoInit(Kernel *pKernel);

  void LoadPlugins();

  static Kernel *CreateInstanceInternal(Slice<const KeyValuePair> commandLine);
  virtual epResult InitInternal() = 0;

  void* CreateImpl(String componentType, Component *pInstance, Variant::VarMap initParams) override final;

  epResult InitComponents();
  epResult InitRender();
  epResult DeinitRender();

  epResult DestroyComponent(Component *pInstance);

  epResult ReceiveMessage(String sender, String message, const Variant &data);

  int SendMessage(LuaState L);

  void Update();
  void StreamerUpdate();

  void RelayStdIO();

  template<typename CT>
  static Component *NewComponent(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams);
};


template<typename ...Args>
inline void Kernel::LogError(String text, Args... args) const
{
  if (sizeof...(Args) == 0)
    spLogger->Log(LogDefaults::LogLevel, text, LogCategories::Error, nullptr);
  else
  {
    MutableString128 tmp(Format, text, args...);
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
    MutableString128 tmp(Format, text, args...);
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
    MutableString128 tmp(Format, text, args...);
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
    MutableString128 tmp(Format, text, args...);
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
    MutableString128 tmp(Format, text, args...);
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
    MutableString128 tmp(Format, text, args...);
    spLogger->Log(LogDefaults::LogLevel, tmp, LogCategories::Trace, nullptr);
  }
}

} // namespace kernel

#endif // EPKERNEL_H
