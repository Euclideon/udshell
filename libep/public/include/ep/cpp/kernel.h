#include "ep/cpp/component/component.h"

#if !defined(_EP_KERNEL_HPP)
#define _EP_KERNEL_HPP

#include "ep/cpp/internal/i/ikernel.h"

namespace ep {

class KernelImpl;
SHARED_CLASS(DynamicComponent);

class Kernel : public Component
{
  __EP_DECLARE_COMPONENT_STATIC_IMPL(ep, Kernel, IKernel, IKernelStatic, Component, EPKERNEL_PLUGINVERSION, "Kernel instance", 0)
public:
  static Kernel* createInstance(Variant::VarMap commandLine, int renderThreadCount);

  static Kernel* getInstance();

  void sendMessage(String target, String sender, String message, const Variant &data) { pImpl->sendMessage(target, sender, message, data); }

  template<typename ComponentType, typename Impl = void, typename GlueType = void, typename StaticImpl = void>
  const ComponentDesc* registerComponentType();
  const ComponentDesc* registerComponentType(Variant::VarMap typeDesc) { return pImpl->RegisterComponentTypeFromMap(typeDesc); }

  const ComponentDesc* getComponentDesc(String id) { return pImpl->GetComponentDesc(id); }

  template<typename CT>
  Array<const ep::ComponentDesc *> getDerivedComponentDescs(bool bIncludeBase);
  Array<const ep::ComponentDesc *> getDerivedComponentDescs(String id, bool bIncludeBase) { return pImpl->GetDerivedComponentDescsFromString(id, bIncludeBase); }
  Array<const ep::ComponentDesc *> getDerivedComponentDescs(const ep::ComponentDesc *pBase, bool bIncludeBase) { return pImpl->GetDerivedComponentDescs(pBase, bIncludeBase); }

  void registerMessageHandler(SharedString _name, MessageHandler messageHandler) { pImpl->RegisterMessageHandler(_name, messageHandler); }
  bool hasMessageHandler(SharedString _name) { return pImpl->HasMessageHandler(_name); }

  ComponentRef createComponent(String typeId, Variant::VarMap initParams = nullptr) { return pImpl->CreateComponent(typeId, initParams); }
  template<typename T>
  SharedPtr<T> createComponent(Variant::VarMap initParams = nullptr);

  ComponentRef createGlue(String typeId, const ComponentDesc *_pType, SharedString _uid, ComponentRef spInstance, Variant::VarMap initParams) { return pImpl->CreateGlue(typeId, _pType, _uid, spInstance, initParams); }
  template<typename T>
  SharedPtr<T> createGlue(const ComponentDesc *_pType, SharedString _uid, ComponentRef spInstance, Variant::VarMap initParams);

  ComponentRef findComponent(String _uid) const { return pImpl->FindComponent(_uid); }

  // synchronisation
  virtual void dispatchToMainThread(MainThreadCallback callback) { pImpl->DispatchToMainThread(callback); }
  virtual void dispatchToMainThreadAndWait(MainThreadCallback callback) { pImpl->DispatchToMainThreadAndWait(callback); }

  LuaRef getLua() const { return pImpl->GetLua(); }
  void exec(String code) { pImpl->Exec(code); }

  LoggerRef getLogger() const { return pImpl->GetLogger(); }
  void log(int kind, int level, String text, String component = nullptr) const { pImpl->Log(kind, level, text, component); }
  template<typename ...Args> void logError(String format, Args... args) const;
  template<typename ...Args> void logWarning(int level, String format, Args... args) const;
  template<typename ...Args> void logDebug(int level, String format, Args... args) const;
  template<typename ...Args> void logInfo(int level, String format, Args... args) const;
  template<typename ...Args> void logScript(String format, Args... args) const;
  template<typename ...Args> void logTrace(String format, Args... args) const;

  // Functions for resource management
  ResourceManagerRef getResourceManager() const { return pImpl->GetResourceManager(); }

  const AVLTree<String, const ComponentDesc *> &getExtensionsRegistry() const { return pImpl->GetExtensionsRegistry(); }
  void registerExtensions(const ComponentDesc *pDesc, Slice<const String> exts) { pImpl->RegisterExtensions(pDesc, exts); }
  DataSourceRef createDataSourceFromExtension(String ext, Variant::VarMap initParams) { return pImpl->CreateDataSourceFromExtension(ext, initParams); }
  DataSourceRef createDataSourceFromFile(String filePath) { return pImpl->CreateDataSourceFromExtension(filePath.getRightAtLast('.'), { { "src", filePath } }); }

  // stdio relaying functions
  BroadcasterRef getStdOutBroadcaster() const { return pImpl->GetStdOutBroadcaster(); }
  BroadcasterRef getStdErrBroadcaster() const { return pImpl->GetStdErrBroadcaster(); }

  // other functions
  ViewRef getFocusView() const { return pImpl->GetFocusView(); }
  virtual ViewRef setFocusView(ViewRef spView) { return pImpl->SetFocusView(spView); }

  CommandManagerRef getCommandManager() const { return pImpl->GetCommandManager(); }

  SettingsRef getSettings() const { return pImpl->GetSettings(); }

  // events
  Event<double> updatePulse;

  // HACK: we might be able to make better paths to this
  KernelImpl* getImpl() const { return (KernelImpl*)pImpl.ptr(); }


  // static methods
  static void setEnvironmentVar(String name, String value) { return getStaticImpl()->SetEnvironmentVar(name, value); }
  static MutableString<0> getEnvironmentVar(String name) { return getStaticImpl()->GetEnvironmentVar(name); }
  static MutableString<0> resolveString(String string, bool bRecursive = true) { return getStaticImpl()->ResolveString(string, bRecursive); }


  // *** these are for internal use ***
  virtual void runMainLoop() = 0;
  virtual void quit() {}

  Array<const PropertyInfo> getProperties() const;
  Array<const MethodInfo> getMethods() const;
  Array<const EventInfo> getEvents() const;
  Array<const StaticFuncInfo> getStaticFuncs() const;

protected:
  Kernel(ComponentDescInl *_pType, Variant::VarMap commandLine);
  ~Kernel();

  //! \cond
  void finishInit() { pImpl->FinishInit(); }
  //! \endcond

  // HACK: This is a temporary method to let us handle fatals; this should probably be moved to a separate component
  virtual void onFatal(String epUnusedParam(msg)) {}

private:
  friend class Component;
  friend class ComponentImpl;

  const ComponentDesc* registerComponentType(ComponentDescInl *pDesc) { return pImpl->RegisterComponentType(pDesc); }

  template<typename GlueType>
  void registerGlueType();
  template<typename StaticImpl, typename ComponentType>
  struct CreateStaticImpl;

  static Kernel* createInstanceInternal(Variant::VarMap commandLine);

  void* createImpl(String componentType, Component *pInstance, Variant::VarMap initParams) { return pImpl->CreateImpl(componentType, pInstance, initParams); }

  static ComponentDescInl *makeKernelDescriptor(ComponentDescInl *pType);
};

} // namespace ep

#include "ep/cpp/internal/kernel_inl.h"

#endif // _EP_KERNEL_HPP
