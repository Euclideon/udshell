#include "ep/cpp/component/component.h"

#if !defined(_EP_KERNEL_HPP)
#define _EP_KERNEL_HPP

#include "ep/cpp/internal/i/ikernel.h"

namespace ep {

class KernelImpl;

class Kernel : public Component, public IKernel
{
  __EP_DECLARE_COMPONENT_IMPL(Kernel, IKernel, Component, EPKERNEL_PLUGINVERSION, "Kernel instance")
public:
  static Kernel* CreateInstance(Variant::VarMap commandLine, int renderThreadCount);

  static Kernel* GetInstance();

  void SendMessage(String target, String sender, String message, const Variant &data) override final { pImpl->SendMessage(target, sender, message, data); }

  template<typename ComponentType, typename Impl = void>
  const ComponentDesc* RegisterComponentType();
  const ComponentDesc* RegisterComponentType(Variant::VarMap typeDesc) override final { return pImpl->RegisterComponentType(typeDesc); }

  template<typename GlueType>
  void RegisterGlueType();

  const ComponentDesc* GetComponentDesc(String id) override final { return pImpl->GetComponentDesc(id); }

  template<typename CT>
  Array<const ep::ComponentDesc *> GetDerivedComponentDescs(bool bIncludeBase);
  Array<const ep::ComponentDesc *> GetDerivedComponentDescs(String id, bool bIncludeBase) override final { return pImpl->GetDerivedComponentDescs(id, bIncludeBase); }
  Array<const ep::ComponentDesc *> GetDerivedComponentDescs(const ep::ComponentDesc *pBase, bool bIncludeBase) override final { return pImpl->GetDerivedComponentDescs(pBase, bIncludeBase); }

  void RegisterMessageHandler(SharedString _name, MessageHandler messageHandler) override final { pImpl->RegisterMessageHandler(_name, messageHandler); }

  ComponentRef CreateComponent(String typeId, Variant::VarMap initParams) override final { return pImpl->CreateComponent(typeId, initParams); }
  template<typename T>
  SharedPtr<T> CreateComponent(Variant::VarMap initParams = nullptr);

  ComponentRef CreateGlue(String typeId, const ComponentDesc *_pType, SharedString _uid, Variant::VarMap initParams) override final { return pImpl->CreateGlue(typeId, _pType, _uid, initParams); }
  template<typename T>
  SharedPtr<T> CreateGlue(const ComponentDesc *_pType, SharedString _uid, Variant::VarMap initParams);

  ComponentRef FindComponent(String _uid) const override final { return pImpl->FindComponent(_uid); }

  // synchronisation
  void DispatchToMainThread(MainThreadCallback callback) override { pImpl->DispatchToMainThread(callback); }
  void DispatchToMainThreadAndWait(MainThreadCallback callback) override { pImpl->DispatchToMainThreadAndWait(callback); }

  LuaRef GetLua() const override final { return pImpl->GetLua(); }
  void Exec(String code) override final { pImpl->Exec(code); }

  LoggerRef GetLogger() const override final { return pImpl->GetLogger(); }
  void Log(int kind, int level, String text, String component = nullptr) const override final { pImpl->Log(kind, level, text, component); }
  template<typename ...Args> void LogError(String format, Args... args) const;
  template<typename ...Args> void LogWarning(int level, String format, Args... args) const;
  template<typename ...Args> void LogDebug(int level, String format, Args... args) const;
  template<typename ...Args> void LogInfo(int level, String format, Args... args) const;
  template<typename ...Args> void LogScript(String format, Args... args) const;
  template<typename ...Args> void LogTrace(String format, Args... args) const;

  // Functions for resource management
  ResourceManagerRef GetResourceManager() const override final { return pImpl->GetResourceManager(); }

  const AVLTree<String, const ComponentDesc *> &GetExtensionsRegistry() const override final { return pImpl->GetExtensionsRegistry(); }
  void RegisterExtensions(const ComponentDesc *pDesc, const Slice<const String> exts) override final { pImpl->RegisterExtensions(pDesc, exts); }
  DataSourceRef CreateDataSourceFromExtension(String ext, Variant::VarMap initParams) override final { return pImpl->CreateDataSourceFromExtension(ext, initParams); }

  // stdio relaying functions
  BroadcasterRef GetStdOutBroadcaster() const override final { return pImpl->GetStdOutBroadcaster(); }
  BroadcasterRef GetStdErrBroadcaster() const override final { return pImpl->GetStdErrBroadcaster(); }

  // other functions
  ViewRef GetFocusView() const override final { return pImpl->GetFocusView(); }
  ViewRef SetFocusView(ViewRef spView) override { return pImpl->SetFocusView(spView); }

  CommandManagerRef GetCommandManager() const override final { return pImpl->GetCommandManager(); }

  // events
  Event<double> UpdatePulse;

  // HACK: we might be able to make better paths to this
  KernelImpl* GetImpl() const { return (KernelImpl*)pImpl.ptr(); }

  // *** these are for internal use ***
  virtual void RunMainLoop() = 0;
  virtual void Quit() {}

protected:
  Kernel(ComponentDescInl *_pType, Variant::VarMap commandLine);
  ~Kernel();

  void FinishInit() override { pImpl->FinishInit(); }

  const ComponentDesc* RegisterComponentType(ComponentDescInl *pDesc) override final { return pImpl->RegisterComponentType(pDesc); }
  void RegisterGlueType(String _name, CreateGlueFunc *pCreateFunc) override final { pImpl->RegisterGlueType(_name, pCreateFunc); }

  template<typename ComponentType, typename Impl = void>
  struct CreateHelper;

private:
  friend class Component;
  friend class ComponentImpl;

  static Kernel* CreateInstanceInternal(Variant::VarMap commandLine);

  void* CreateImpl(String componentType, Component *pInstance, Variant::VarMap initParams) override final { return pImpl->CreateImpl(componentType, pInstance, initParams); }

  static ComponentDescInl *MakeKernelDescriptor(ComponentDescInl *pType);
};

} // namespace ep

#include "ep/cpp/internal/kernel_inl.h"

#endif // _EP_KERNEL_HPP
