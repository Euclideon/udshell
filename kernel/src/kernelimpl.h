#pragma once
#ifndef EPKERNELIMPL_H
#define EPKERNELIMPL_H

#include "components/componentimpl.h"
#include "components/lua.h"
#include "components/logger.h"

#include "ep/cpp/hashmap.h"
#include "ep/cpp/kernel.h"
#include "ep/cpp/component/timer.h"

#include "ep/cpp/internal/i/ikernel.h"

#include "ep/cpp/internal/fastdelegate.h"
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

class KernelImpl : public BaseImpl<Kernel, IKernel>
{
public:
  KernelImpl(Kernel *pInstance, Variant::VarMap initParams);

  template <typename CT, typename ...Args>
  static CT* CreateComponentInstance(Args ...args);

  bool sendMessage(String target, String sender, String message, const Variant &data) override final;

  void RegisterMessageHandler(SharedString name, MessageHandler messageHandler) override final;
  bool HasMessageHandler(SharedString _name) override final { return messageHandlers.get(_name) != nullptr; }

  // synchronisation
  void DispatchToMainThread(MainThreadCallback callback) override final;
  void DispatchToMainThreadAndWait(MainThreadCallback callback) override final;

  // component registry
  const ComponentDesc* RegisterComponentType(ComponentDescInl *pDesc) override final;
  const ComponentDesc* RegisterComponentTypeFromMap(Variant::VarMap typeDesc) override final;

  void RegisterGlueType(String name, CreateGlueFunc *pCreateFunc) override final;

  const ComponentDesc* GetComponentDesc(String id) override final;

  Array<const ep::ComponentDesc *> GetDerivedComponentDescsFromString(String id, bool bIncludeBase) override final;
  Array<const ep::ComponentDesc *> GetDerivedComponentDescs(const ep::ComponentDesc *pBase, bool bIncludeBase) override final;

  ComponentRef CreateComponent(String typeId, Variant::VarMap initParams) override final;
  ComponentRef CreateGlue(String typeId, const ComponentDesc *_pType, SharedString _uid, ComponentRef instance, Variant::VarMap initParams) override final;

  ComponentRef FindComponent(String name) const override final;

  SharedPtr<Renderer> GetRenderer() const;

  // script
  LuaRef GetLua() const override final { return spLua; }
  void Exec(String code) override final;

  // logger functions
  LoggerRef GetLogger() const override final { return spLogger; }
  void Log(int kind, int level, String text, String component = nullptr) const override final;

  // Functions for resource management
  ResourceManagerRef GetResourceManager() const override final;

  const AVLTree<String, const ComponentDesc *> &GetExtensionsRegistry() const override final;
  void RegisterExtensions(const ComponentDesc *pDesc, const Slice<const String> exts) override final;
  DataSourceRef CreateDataSourceFromExtension(String ext, Variant::VarMap initParams) override final;

  // stdio relaying functions
  BroadcasterRef GetStdOutBroadcaster() const override final;
  BroadcasterRef GetStdErrBroadcaster() const override final;

  // other functions
  ViewRef GetFocusView() const override final;
  ViewRef SetFocusView(ViewRef spView) override final;

  CommandManagerRef GetCommandManager() const override final;

  SettingsRef GetSettings() const override final;

  void InitRender();
  void DeinitRender();

  void Shutdown();

//protected:
//  friend class Kernel;
//  friend class Component;
//  friend class ComponentImpl;

  struct ComponentType
  {
    const ComponentDescInl *pDesc;
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

  HashMap<SharedString, ComponentType> componentRegistry;
  HashMap<SharedString, CreateGlueFunc*> glueRegistry;
  HashMap<SharedString, Component*> instanceRegistry;
  HashMap<SharedString, Component*> namedInstanceRegistry;
  HashMap<SharedString, ForeignInstance> foreignInstanceRegistry;
  HashMap<SharedString, MessageCallback> messageHandlers;

  AVLTree<String, const ComponentDesc *> extensionsRegistry;

  Variant::VarMap commandLineArgs;

  SharedPtr<Renderer> spRenderer;
  int renderThreadCount;

  LuaRef spLua;

  LoggerRef spLogger;
  PluginManagerRef spPluginManager;
  ResourceManagerRef spResourceManager;
  CommandManagerRef spCommandManager;
  SettingsRef spSettings;
  ViewRef spFocusView;
  TimerRef spStreamerTimer;
  TimerRef spUpdateTimer;
  BroadcasterRef spStdOutBC;
  BroadcasterRef spStdErrBC;

  StdCapture *stdOutCapture = nullptr;
  StdCapture *stdErrCapture = nullptr;

  using VarAVLTreeAllocator = AVLTreeAllocator<VariantAVLNode>;
  using WeakRefRegistryMap = HashMap<void*, internal::SafeProxy<void>*, internal::PointerHash>;
  using StaticImplRegistryMap = HashMap<SharedString, UniquePtr<RefCounted>>;

  static VarAVLTreeAllocator *s_pVarAVLAllocator;
  static WeakRefRegistryMap *s_pWeakRefRegistry;
  static StaticImplRegistryMap *s_pStaticImplRegistry;

  bool bKernelCreated = false;

  virtual ~KernelImpl();

  void* CreateImpl(String componentType, Component *pInstance, Variant::VarMap initParams) override final;

  void StartInit(Variant::VarMap initParams);
  void FinishInit() override final;
  void InitComponents();
  void LoadAllPlugins(Slice<const String> folderPaths);
  void LoadPlugins(Slice<SharedString> files);
  Array<SharedString> ScanPluginFolder(String folderPath, Slice<const String> extfilter = nullptr);

  void DestroyComponent(Component *pInstance);

  void ReceiveMessage(String sender, String message, const Variant &data);

  int sendMessage(LuaState L);

  void Update();
  void StreamerUpdate();

  void RelayStdIO();

  void WrangleEnvironmentVariables();

  template<typename CT>
  static Component *NewComponent(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams);
};

class KernelImplStatic : public BaseStaticImpl<IKernelStatic>
{
  void SetEnvironmentVar(String name, String value) override final;
  MutableString<0> GetEnvironmentVar(String name) override final;
  MutableString<0> ResolveString(String string, bool bRecursive) override final;
};


template <typename CT, typename ...Args>
CT* KernelImpl::CreateComponentInstance(Args ...args)
{
  void *pMem = epAlloc(sizeof(CT));
  epscope(fail) { if (pMem) epFree(pMem); };
  EPTHROW_IF_NULL(pMem, Result::AllocFailure, "Memory allocation failed");

  CT *pNew = epConstruct (pMem) CT(args...);
  pNew->pFreeFunc = [](RefCounted *pMem) { epFree((CT*)pMem); };

  return pNew;
}

} // namespace ep

#endif // EPKERNELIMPL_H
