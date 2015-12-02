#if !defined(_EP_KERNEL_HPP)
#define _EP_KERNEL_HPP

#include "ep/cpp/variant.h"
#include "ep/cpp/component.h"
#include "ep/cpp/interface/icomponent.h"

namespace kernel {
struct ComponentDesc;
}

namespace ep {

class Kernel
{
public:
  static Kernel* GetInstance();

  virtual epResult SendMessage(String target, String sender, String message, const Variant &data) = 0;

  template<typename ComponentType>
  epResult RegisterComponentType();

  virtual const ComponentDesc* GetComponentDesc(String id) = 0;

  typedef FastDelegate<void(String sender, String message, const Variant &data)> MessageHandler;
  virtual void RegisterMessageHandler(SharedString name, MessageHandler messageHandler) = 0;

  virtual epResult CreateComponent(String typeId, Variant::VarMap initParams, ComponentRef *pNewInstance) = 0;
  template<typename T>
  SharedPtr<T> CreateComponent(Variant::VarMap initParams = nullptr);

  virtual ComponentRef FindComponent(String uid) const = 0;

  virtual epResult RegisterExtensions(const ep::ComponentDesc *pDesc, const Slice<const String> exts) = 0;
//  virtual DataSourceRef CreateDataSourceFromExtension(String ext, Variant::VarMap initParams) = 0;

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

protected:
  virtual const kernel::ComponentDesc* RegisterComponentType(const ComponentDesc &desc) = 0;

  template<typename ComponentType, bool IsIComponent>
  struct CreateHelper;
};

} // namespace ep

#include "ep/cpp/internal/kernel_inl.h"

#endif // _EP_KERNEL_HPP
