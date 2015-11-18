#pragma once
#if !defined(_EP_COMPONENT_HPP)
#define _EP_COMPONENT_HPP

#include "ep/c/plugin.h"
#include "ep/cpp/variant.h"
#include "ep/c/componentdesc.h"
#include "ep/c/internal/component_inl.h"

namespace ep {

// component API
class Component final : public RefCounted
{
public:
  SharedString GetUID() const;
  SharedString GetName() const;

  bool IsType(String type) const;

  Variant GetProperty(String property) const;
  void SetProperty(String property, const Variant &value);

  Variant CallMethod(String method, Slice<const Variant> args);
  template<typename ...Args>
  Variant CallMethod(String method, Args... args);

  void Subscribe(String eventName, Variant::VarDelegate delegate);

  epResult SendMessage(String target, String message, const Variant &data);

private:
  const epComponentDesc *pType;
  class Kernel *pKernel;

  const SharedString uid;
  SharedString name;

  void *pUserData;
};

ptrdiff_t epStringify(Slice<char> buffer, String format, Component *pComponent, const epVarArg *pArgs);

} // namespace ep

#include "ep/cpp/internal/component_inl.h"

#endif
