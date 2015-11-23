#if !defined(_EP_KERNEL_HPP)
#define _EP_KERNEL_HPP

#include "ep/cpp/variant.h"
#include "ep/cpp/component.h"

namespace ep {

class Kernel
{
public:
  static Kernel* GetInstance();

  virtual epResult SendMessage(String target, String sender, String message, const Variant &data) = 0;

  template<typename ComponentType>
  epResult RegisterComponentType();

  typedef FastDelegate<void(String sender, String message, const Variant &data)> MessageHandler;
  virtual void RegisterMessageHandler(SharedString name, MessageHandler messageHandler) = 0;

  virtual epResult CreateComponent(String typeId, InitParams initParams, ComponentRef *pNewInstance) = 0;

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
};

} // namespace ep

#include "ep/cpp/internal/kernel_inl.h"

#endif // _EP_KERNEL_HPP
