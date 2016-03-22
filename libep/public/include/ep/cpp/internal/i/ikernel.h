#pragma once
#if !defined(_EP_IKERNEL_HPP)
#define _EP_IKERNEL_HPP

#include "ep/cpp/internal/fastdelegate.h"
#include "ep/cpp/variant.h"
#include "ep/cpp/componentdesc.h"

using namespace fastdelegate;

namespace ep {

typedef FastDelegate<void(String sender, String message, const Variant &data)> MessageHandler;
typedef FastDelegate<void()> MainThreadCallback;
typedef ComponentRef(CreateGlueFunc)(Kernel *pKernel, const ComponentDesc *_pType, SharedString _uid, ComponentRef spInstance, Variant::VarMap initParams);

SHARED_CLASS(ResourceManager);
SHARED_CLASS(CommandManager);
SHARED_CLASS(Broadcaster);
SHARED_CLASS(DataSource);
SHARED_CLASS(Logger);
SHARED_CLASS(View);
SHARED_CLASS(Lua);

class IKernel
{
public:
  virtual const ComponentDesc* RegisterComponentType(ComponentDescInl *pDesc) = 0;
  virtual const ComponentDesc* RegisterComponentType(Variant::VarMap typeDesc) = 0;

  virtual void RegisterGlueType(String name, CreateGlueFunc *pCreateFunc) = 0;

  virtual void* CreateImpl(String componentType, Component *pInstance, Variant::VarMap initParams) = 0;

  virtual void SendMessage(String target, String sender, String message, const Variant &data) = 0;

  virtual void RegisterMessageHandler(SharedString name, MessageHandler messageHandler) = 0;

  virtual void DispatchToMainThread(MainThreadCallback callback) = 0;
  virtual void DispatchToMainThreadAndWait(MainThreadCallback callback) = 0;

  virtual const ComponentDesc* GetComponentDesc(String id) = 0;

  virtual Array<const ep::ComponentDesc *> GetDerivedComponentDescs(String id, bool bIncludeBase) = 0;
  virtual Array<const ep::ComponentDesc *> GetDerivedComponentDescs(const ep::ComponentDesc *pBase, bool bIncludeBase) = 0;

  virtual ComponentRef CreateComponent(String typeId, Variant::VarMap initParams) = 0;

  virtual ComponentRef CreateGlue(String typeId, const ComponentDesc *_pType, SharedString _uid, ComponentRef spInstance, Variant::VarMap initParams) = 0;

  virtual ComponentRef FindComponent(String name) const = 0;

  virtual LuaRef GetLua() const = 0;
  virtual void Exec(String code) = 0;

  virtual LoggerRef GetLogger() const = 0;
  virtual void Log(int kind, int level, String text, String component) const = 0;

  virtual ResourceManagerRef GetResourceManager() const = 0;

  virtual const AVLTree<String, const ComponentDesc *> &GetExtensionsRegistry() const = 0;
  virtual void RegisterExtensions(const ComponentDesc *pDesc, const Slice<const String> exts) = 0;
  virtual DataSourceRef CreateDataSourceFromExtension(String ext, Variant::VarMap initParams) = 0;

  // stdio relaying functions
  virtual BroadcasterRef GetStdOutBroadcaster() const = 0;
  virtual BroadcasterRef GetStdErrBroadcaster() const = 0;

  // other functions
  virtual ViewRef GetFocusView() const = 0;
  virtual ViewRef SetFocusView(ViewRef spView) = 0;

  virtual CommandManagerRef GetCommandManager() const = 0;

  virtual void FinishInit() = 0;
};

} // namespace ep

#endif // _EP_IKERNEL_HPP
