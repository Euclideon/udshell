#pragma once
#if !defined(_EP_COMPONENT_HPP)
#define _EP_COMPONENT_HPP

#include "ep/c/plugin.h"
#include "ep/cpp/variant.h"
#include "ep/c/componentdesc.h"
#include "ep/c/internal/component_inl.h"

namespace kernel {
struct ComponentDesc;
class Kernel;
}

namespace ep {

// component API
class Component : public RefCounted
{
public:
  const SharedString uid;
  SharedString name;

  Kernel* GetKernel() const;

  SharedString GetUid() const;
  SharedString GetName() const;
  void SetName(SharedString _name);

  bool IsType(String type) const;

  virtual Variant GetProperty(String property) const = 0;
  virtual void SetProperty(String property, const Variant &value) = 0;

  virtual Variant CallMethod(String method, Slice<const Variant> args) = 0;
  template<typename ...Args>
  Variant CallMethod(String method, Args... args);

  virtual void Subscribe(String eventName, const Variant::VarDelegate &delegate) = 0;

  virtual epResult SendMessage(String target, String message, const Variant &data) = 0;

  virtual Variant Save() const = 0;

  template<typename ...Args> void LogError(String text, Args... args) const;
  template<typename ...Args> void LogWarning(int level, String text, Args... args) const;
  template<typename ...Args> void LogDebug(int level, String text, Args... args) const;
  template<typename ...Args> void LogInfo(int level, String text, Args... args) const;
  template<typename ...Args> void LogScript(String text, Args... args) const;
  template<typename ...Args> void LogTrace(String text, Args... args) const;

  void* GetUserData() const;

protected:
  Component(const kernel::ComponentDesc *_pType, kernel::Kernel *_pKernel, SharedString _uid, InitParams initParams)
    : uid(_uid), pType(_pType), pKernel(_pKernel) {}

  const kernel::ComponentDesc *const pType;
  class kernel::Kernel *const pKernel;

  void *pUserData = nullptr;

  void LogInternal(int category, int level, String text, String componentUID) const;
};

ptrdiff_t epStringify(Slice<char> buffer, String format, Component *pComponent, const epVarArg *pArgs);

} // namespace ep

#include "ep/cpp/internal/component_inl.h"

#endif
