#pragma once
#if !defined(_EP_INTERFACE_ICOMPONENT_HPP)
#define _EP_INTERFACE_ICOMPONENT_HPP

#include "ep/cpp/component.h"
#include "ep/cpp/sharedptr.h"

namespace ep {

class IComponent
{
  EP_DECLARE_COMPONENT(IComponent, IComponent, 100, "Component Interface")
public:
  SharedString GetUid() const                                               { return component.GetUid(); }
  SharedString GetName() const                                              { return component.GetName(); }

  bool IsType(String type) const                                            { return component.IsType(type); }

  Variant GetProperty(String property) const                                { return component.GetProperty(property); }
  void SetProperty(String property, const Variant &value)                   { component.SetProperty(property, value); }

  Variant CallMethod(String method, Slice<const Variant> args)              { return component.CallMethod(method, args); }
  template<typename ...Args>
  Variant CallMethod(String method, Args... args)
  {
    const Variant varargs[sizeof...(Args)+1] = { args... };
    return component.CallMethod(method, Slice<const Variant>(varargs, sizeof...(Args)));
  }

  void Subscribe(String eventName, Variant::VarDelegate delegate)           { component.Subscribe(eventName, delegate); }

  epResult SendMessage(String target, String message, const Variant &data)  { return component.SendMessage(target, message, data); }

  virtual epResult InitComplete()                                                     { return epR_Success; }
  virtual epResult ReceiveMessage(String message, String sender, const Variant &data) { return epR_Success; }

  operator ComponentRef() const                                             { return (ComponentRef&)component; }

protected:
  IComponent(Component &baseComponent, Slice<const KeyValuePair> initParams) : component(baseComponent) {}
  virtual ~IComponent() {}

  Component &component;

  // component registration
  static epComponentOverrides GetOverrides();

private:
  template<typename T>
  static void* CreateInstance(epComponent *pBaseInstance, const epKeyValuePair *pInitParams, size_t numInitParams)
  {
    return new T((Component*&)pBaseInstance, Slice<KeyValuePair>((KeyValuePair*)pInitParams, numInitParams));
  }
};

} // namespace ep

#endif // _EP_INTERFACE_ICOMPONENT_HPP
