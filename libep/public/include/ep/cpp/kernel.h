#if !defined(_EP_KERNEL_HPP)
#define _EP_KERNEL_HPP

#include "ep/cpp/variant.h"
#include "ep/cpp/component.h"

namespace ep {

class Kernel final
{
public:
  static Kernel* GetInstance();

  epResult SendMessage(String target, String sender, String message, const Variant &data);

  template<typename ComponentType>
  epResult RegisterComponentType();
  epResult CreateComponent(String typeId, Slice<const KeyValuePair> initParams, Component **ppNewInstance);

  epComponent* FindComponent(String uid) const;

  void Exec(String code);

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
