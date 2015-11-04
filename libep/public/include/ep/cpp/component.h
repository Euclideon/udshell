#pragma once
#if !defined(_EP_COMPONENT_HPP)
#define _EP_COMPONENT_HPP

#include "ep/c/plugin.h"
#include "ep/cpp/variant.h"
#include "ep/c/componentdesc.h"
#include "ep/c/component.h"

namespace ep {

// component API
class Component : protected epComponent
{
public:
  String GetUID() const                                                     { return (String&)uid; }
  String GetName() const                                                    { return s_pPluginInstance->pComponentAPI->GetName((const epComponent*)this); }

  bool IsType(String type) const                                            { return s_pPluginInstance->pComponentAPI->IsType((const epComponent*)this, type); }

  Variant GetProperty(String property) const                                { return s_pPluginInstance->pComponentAPI->GetProperty((const epComponent*)this, property); }
  void SetProperty(String property, const Variant &value)                   { s_pPluginInstance->pComponentAPI->SetProperty((epComponent*)this, property, (const epVariant*)&value); }

  Variant CallMethod(String method, Slice<const Variant> args)              { return s_pPluginInstance->pComponentAPI->CallMethod((epComponent*)this, method, (const epVariant*)args.ptr, args.length); }
  template<typename ...Args>
  Variant CallMethod(String method, Args... args)
  {
    const Variant varargs[sizeof...(Args)+1] = { args... };
    return CallMethod(method, Slice<const Variant>(varargs, sizeof...(Args)));
  }

  void Subscribe(String eventName, Variant::VarDelegate delegate)           { s_pPluginInstance->pComponentAPI->Subscribe((epComponent*)this, eventName, (const epVarDelegate*&)delegate); }

  epResult SendMessage(String target, String message, const Variant &data)  { return s_pPluginInstance->pComponentAPI->SendMessage((epComponent*)this, target, message, (const epVariant*)&data); }
};

ptrdiff_t epStringify(Slice<char> buffer, String format, Component *pComponent, const epVarArg *pArgs);

} // namespace ep


#endif
