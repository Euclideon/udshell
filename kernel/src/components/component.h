#pragma once
#ifndef EPCOMPONENT_H
#define EPCOMPONENT_H

#include "componentdesc.h"
#include "ep/cpp/event.h"
#include "ep/cpp/component.h"

#include "udHashMap.h"

#include <stdio.h>

#if defined(SendMessage)
# undef SendMessage
#endif


#define EP_COMPONENT(Name) \
  friend class ::kernel::Kernel; \
  static ::kernel::ComponentDesc descriptor; \
  typedef ::ep::SharedPtr<Name> Ref;

namespace kernel {

class Component : public ep::Component
{
public:
  EP_COMPONENT(Component);

  const ComponentDesc* GetDescriptor() const { return pType; }

  template<typename T>
  bool IsType() const             { return ep::Component::IsType(T::descriptor.id); }
  bool IsType(String type) const  { return ep::Component::IsType(type); }

  Variant GetProperty(String property) const override final;
  void SetProperty(String property, const Variant &value) override final;

  Variant CallMethod(String method, Slice<const Variant> args) override final;
  template<typename ...Args>
  Variant CallMethod(String method, Args... args)
  {
    const Variant varargs[sizeof...(Args)+1] = { args... };
    return CallMethod(method, Slice<const Variant>(varargs, sizeof...(Args)));
  }

  void Subscribe(String eventName, const Variant::VarDelegate &delegate) override final;
  void Unsubscribe();

  // helper for subscribing a shim
  template<typename ...Args>
  void Subscribe(String eventName, const Delegate<void(Args...)> &d);

  // helpers for direct subscription
  template <class X, class Y, typename ...Args>
  void Subscribe(String eventName, Y *pThis, void(X::*pMethod)(Args...))        { Subscribe(eventName, Delegate<void(Args...)>(pThis, pMethod)); }
  template <class X, class Y, typename ...Args>
  void Subscribe(String eventName, Y *pThis, void(X::*pMethod)(Args...) const)  { Subscribe(eventName, Delegate<void(Args...)>(pThis, pMethod)); }
  template <typename ...Args>
  void Subscribe(String eventName, void(*pFunc)(Args...))                       { Subscribe(eventName, Delegate<void(Args...)>(pFunc)); }

  epResult SendMessage(String target, String message, const Variant &data) override final;
  epResult SendMessage(Component *pComponent, String message, const Variant &data) { return SendMessage(MutableString128(Concat, "@", pComponent->uid), message, data); }

  const PropertyInfo *GetPropertyInfo(String propName) const
  {
    const PropertyDesc *pDesc = GetPropertyDesc(propName);
    return pDesc ? &pDesc->info : nullptr;
  }
  const FunctionInfo *GetMethodInfo(String methName) const
  {
    const MethodDesc *pDesc = GetMethodDesc(methName);
    return pDesc ? &pDesc->info : nullptr;
  }
  const EventInfo *GetEventInfo(String eventName) const
  {
    const EventDesc *pDesc = GetEventDesc(eventName);
    return pDesc ? &pDesc->info : nullptr;
  }
  const FunctionInfo *GetStaticFuncInfo(String staticFuncName) const
  {
    const StaticFuncDesc *pDesc = GetStaticFuncDesc(staticFuncName);
    return pDesc ? &pDesc->info : nullptr;
  }

  // built-in component properties
  String GetType() const { return pType->id; }
  String GetDisplayName() const { return pType->displayName; }
  String GetDescription() const { return pType->description; }

  void AddDynamicProperty(const PropertyDesc &property);
  void AddDynamicMethod(const MethodDesc &method);
  void AddDynamicEvent(const EventDesc &event);
  void RemoveDynamicProperty(String name);
  void RemoveDynamicMethod(String name);
  void RemoveDynamicEvent(String name);

  Variant Save() const override { return Variant(); }

protected:
  Component(const ComponentDesc *_pType, Kernel *_pKernel, SharedString _uid, InitParams initParams)
    : ep::Component(_pType, _pKernel, _uid, initParams) {}
  virtual ~Component();

  void operator delete(void *p)
  {
    ::operator delete(p);
  }

  void Init(InitParams initParams);
  virtual epResult InitComplete() { return epR_Success; }

  virtual epResult ReceiveMessage(String message, String sender, const Variant &data);

  // property access
  size_t NumProperties() const { return instanceProperties.Size() + pType->propertyTree.Size(); }
  size_t NumMethods() const { return instanceMethods.Size() + pType->methodTree.Size(); }
  size_t NumEvents() const { return instanceEvents.Size() + pType->eventTree.Size(); }
  size_t NumStaticFuncs() const { return pType->staticFuncTree.Size(); }

  const PropertyDesc *GetPropertyDesc(String name) const;
  const MethodDesc *GetMethodDesc(String name) const;
  const EventDesc *GetEventDesc(String name) const;
  const StaticFuncDesc *GetStaticFuncDesc(String name) const;

  // TODO: these substantially inflate the size of base Component and are almost always nullptr
  // ...should we move them to a separate allocation?
  AVLTree<SharedString, PropertyDesc> instanceProperties;
  AVLTree<SharedString, MethodDesc> instanceMethods;
  AVLTree<SharedString, EventDesc> instanceEvents;

private:
  template<typename... Args>
  friend class Event;
  friend class LuaState;

  Component(const Component &) = delete;    // Still not sold on this
  void operator=(const Component &) = delete;
};

} // namespace kernel

// HAX: this needs to be here for stupid C++ forward referencing reasons
#include "components/logger.h"

namespace kernel {

// inlines...
template<typename ...Args>
inline void Component::Subscribe(String eventName, const Delegate<void(Args...)> &d)
{
  typedef SharedPtr<internal::VarDelegateMemento<void(Args...)>> VarDelegateMementoRef;
  Subscribe(eventName, Variant::VarDelegate(VarDelegateMementoRef::create(d)));
}

template<typename T>
inline SharedPtr<T> component_cast(ComponentRef pComponent)
{
  if (!pComponent)
    return nullptr;
  const ComponentDesc *pDesc = pComponent->GetDescriptor();
  while (pDesc)
  {
    if (pDesc->id.eq(T::descriptor.id))
      return shared_pointer_cast<T>(pComponent);
    pDesc = pDesc->pSuperDesc;
  }
  return nullptr;
}

ptrdiff_t epStringify(Slice<char> buffer, String format, const Component *pComponent, const epVarArg *pArgs);

} // namespace kernel

namespace ep {
  
inline Variant epToVariant(ep::Component *pC)
{
  epVariant v;
  v.t = (size_t)Variant::Type::Component;
  v.ownsContent = 0;
  v.length = 0;
  v.p = pC;
  return std::move(v);
}

// HACK: this is here because forward referencing!
// SharedPtr<Component> (and derived types)
template<typename T, typename std::enable_if<std::is_base_of<Component, T>::value>::type* = nullptr> // O_O
inline void epFromVariant(const Variant &v, SharedPtr<T> *pR)
{
  *pR = kernel::component_cast<T>(v.asComponent());
}

}

#endif // EPCOMPONENT_H
