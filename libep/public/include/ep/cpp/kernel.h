#if !defined(_EP_KERNEL_HPP)
#define _EP_KERNEL_HPP

#include "ep/cpp/variant.h"
#include "ep/cpp/componentdesc.h"
#include "ep/cpp/internal/i/icomponent.h"

namespace ep {

SHARED_CLASS(Component);
SHARED_CLASS(View);
SHARED_CLASS(DataSource);
SHARED_CLASS(ResourceManager);
SHARED_CLASS(CommandManager);
SHARED_CLASS(Broadcaster);

class Kernel
{
public:
  static Kernel* GetInstance();

  virtual void SendMessage(String target, String sender, String message, const Variant &data) = 0;

  template<typename ComponentType, typename Impl = void>
  const ComponentDesc* RegisterComponentType();

  virtual const ComponentDesc* GetComponentDesc(String id) = 0;

  typedef FastDelegate<void(String sender, String message, const Variant &data)> MessageHandler;
  virtual void RegisterMessageHandler(SharedString name, MessageHandler messageHandler) = 0;

  virtual ComponentRef CreateComponent(String typeId, Variant::VarMap initParams) = 0;
  template<typename T>
  SharedPtr<T> CreateComponent(Variant::VarMap initParams = nullptr);

  virtual ComponentRef FindComponent(String uid) const = 0;

  // synchronisation
  typedef FastDelegate<void(Kernel*)> MainThreadCallback;
  virtual void DispatchToMainThread(MainThreadCallback callback) = 0;
  virtual void DispatchToMainThreadAndWait(MainThreadCallback callback) = 0;

  virtual void Exec(String code) = 0;

  virtual void Log(int kind, int level, String text, String component = nullptr) const = 0;
  template<typename ...Args> void LogError(String format, Args... args) const;
  template<typename ...Args> void LogWarning(int level, String format, Args... args) const;
  template<typename ...Args> void LogDebug(int level, String format, Args... args) const;
  template<typename ...Args> void LogInfo(int level, String format, Args... args) const;
  template<typename ...Args> void LogScript(String format, Args... args) const;
  template<typename ...Args> void LogTrace(String format, Args... args) const;

  // Functions for resource management
  virtual ResourceManagerRef GetResourceManager() const = 0;

  virtual void RegisterExtensions(const ComponentDesc *pDesc, const Slice<const String> exts) = 0;
  virtual DataSourceRef CreateDataSourceFromExtension(String ext, Variant::VarMap initParams) = 0;

  // stdio relaying functions
  virtual BroadcasterRef GetStdOutBroadcaster() const = 0;
  virtual BroadcasterRef GetStdErrBroadcaster() const = 0;

  // other functions
  virtual ViewRef GetFocusView() const = 0;
  virtual ViewRef SetFocusView(ViewRef spView) = 0;

  virtual CommandManagerRef GetCommandManager() const = 0;

  // events
  Event<double> UpdatePulse;

private:
  friend class Component;

  virtual const ComponentDesc* RegisterComponentType(const ComponentDesc &desc) = 0;
  virtual void* CreateImpl(String componentType, Component *pInstance, Variant::VarMap initParams) = 0;

  template<typename ComponentType, typename Impl = void>
  struct CreateHelper;
};

} // namespace ep

#include "ep/cpp/internal/kernel_inl.h"

#endif // _EP_KERNEL_HPP
