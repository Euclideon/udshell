#pragma once
#ifndef EPCOMPONENT_H
#define EPCOMPONENT_H

#include "componentdesc.h"
#include "ep/cpp/event.h"

#include "udHashMap.h"

#include <stdio.h>

#if defined(SendMessage)
# undef SendMessage
#endif


#define EP_COMPONENT(Name) \
  friend class ::ep::Kernel; \
  static ::ep::ComponentDesc descriptor; \
  typedef ::ep::SharedPtr<Name> Ref;

namespace ep {

class Component : public RefCounted
{
public:
  EP_COMPONENT(Component);

  const ComponentDesc* const pType;
  class Kernel* const pKernel;

  const SharedString uid;
  SharedString name;

  bool IsType(String type) const;
  template<typename T>
  bool IsType() const { return IsType(T::descriptor.id); }

  Variant GetProperty(String property) const;
  void SetProperty(String property, const Variant &value);

  Variant CallMethod(String method, Slice<Variant> args);
  template<typename ...Args>
  Variant CallMethod(String method, Args... args)
  {
    Variant varargs[sizeof...(Args) + 1] = { args... };
    return CallMethod(method, Slice<Variant>(varargs, sizeof...(Args)));
  }

  void Subscribe(String eventName, const Variant::VarDelegate &d);
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

  epResult SendMessage(String target, String message, const Variant &data);
  epResult SendMessage(Component *pComponent, String message, const Variant &data) { MutableString128 temp; temp.concat("@", pComponent->uid); return SendMessage(temp, message, data); }

  const PropertyInfo *GetPropertyInfo(String name) const
  {
    const PropertyDesc *pDesc = GetPropertyDesc(name);
    return pDesc ? &pDesc->info : nullptr;
  }
  const FunctionInfo *GetMethodInfo(String name) const
  {
    const MethodDesc *pDesc = GetMethodDesc(name);
    return pDesc ? &pDesc->info : nullptr;
  }
  const EventInfo *GetEventInfo(String name) const
  {
    const EventDesc *pDesc = GetEventDesc(name);
    return pDesc ? &pDesc->info : nullptr;
  }
  const FunctionInfo *GetStaticFuncInfo(String name) const
  {
    const StaticFuncDesc *pDesc = GetStaticFuncDesc(name);
    return pDesc ? &pDesc->info : nullptr;
  }

  // built-in component properties
  String GetUid() const { return uid; }
  String GetName() const { return name; }
  String GetType() const { return pType->id; }
  String GetDisplayName() const { return pType->displayName; }
  String GetDescription() const { return pType->description; }

  template<typename ...Args> void LogError(String text, Args... args) const;
  template<typename ...Args> void LogWarning(int level, String text, Args... args) const;
  template<typename ...Args> void LogDebug(int level, String text, Args... args) const;
  template<typename ...Args> void LogInfo(int level, String text, Args... args) const;
  template<typename ...Args> void LogScript(String text, Args... args) const;
  template<typename ...Args> void LogTrace(String text, Args... args) const;

  void SetName(String name) { this->name = name; }


  void AddDynamicProperty(const PropertyDesc &property);
  void AddDynamicMethod(const MethodDesc &method);
  void AddDynamicEvent(const EventDesc &event);
  void RemoveDynamicProperty(String name);
  void RemoveDynamicMethod(String name);
  void RemoveDynamicEvent(String name);

  void* GetUserData() const { return pUserData; }

protected:
  Component(const ComponentDesc *_pType, Kernel *_pKernel, SharedString _uid, InitParams initParams)
    : pType(_pType), pKernel(_pKernel), uid(_uid) {}
  virtual ~Component();

  void Init(InitParams initParams);
  virtual epResult InitComplete() { return epR_Success; }

  virtual epResult ReceiveMessage(String message, String sender, const Variant &data);

  void LogInternal(int level, String text, int category, String componentUID) const;

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
  AVLTree<String, PropertyDesc> instanceProperties;
  AVLTree<String, MethodDesc> instanceMethods;
  AVLTree<String, EventDesc> instanceEvents;

  epSubscriber subscriber;

  void *pUserData = nullptr;

private:
  template<typename... Args>
  friend class ::epEvent;
  friend class LuaState;

  Component(const Component &) = delete;    // Still not sold on this
  void operator=(const Component &) = delete;
};

} // namespace ep

// HAX: this needs to be here for stupid C++ forward referencing reasons
#include "components/logger.h"

namespace ep {

// inlines...
template<typename ...Args>
inline void Component::Subscribe(String eventName, const Delegate<void(Args...)> &d)
{
  typedef SharedPtr<internal::VarDelegateMemento<void(Args...)>> VarDelegateMementoRef;
  Subscribe(eventName, Variant::VarDelegate(VarDelegateMementoRef::create(d)));
}

template<typename ...Args>
inline void Component::LogError(String text, Args... args) const
{
  if (sizeof...(Args) == 0)
    LogInternal(LogDefaults::LogLevel, text, LogCategories::Error, uid);
  else
  {
    MutableString128 tmp; tmp.format(text, args...);
    LogInternal(LogDefaults::LogLevel, tmp, LogCategories::Error, uid);
  }
}
template<typename ...Args>
inline void Component::LogWarning(int level, String text, Args... args) const
{
  if (sizeof...(Args) == 0)
    LogInternal(level, text, LogCategories::Warning, uid);
  else
  {
    MutableString128 tmp; tmp.format(text, args...);
    LogInternal(level, tmp, LogCategories::Warning, uid);
  }
}
template<typename ...Args>
inline void Component::LogDebug(int level, String text, Args... args) const
{
  if (sizeof...(Args) == 0)
    LogInternal(level, text, LogCategories::Debug, uid);
  else
  {
    MutableString128 tmp; tmp.format(text, args...);
    LogInternal(level, tmp, LogCategories::Debug, uid);
  }
}
template<typename ...Args>
inline void Component::LogInfo(int level, String text, Args... args) const
{
  if (sizeof...(Args) == 0)
    LogInternal(level, text, LogCategories::Info, uid);
  else
  {
    MutableString128 tmp; tmp.format(text, args...);
    LogInternal(level, tmp, LogCategories::Info, uid);
  }
}
template<typename ...Args>
inline void Component::LogScript(String text, Args... args) const
{
  if (sizeof...(Args) == 0)
    LogInternal(LogDefaults::LogLevel, text, LogCategories::Script, uid);
  else
  {
    MutableString128 tmp; tmp.format(text, args...);
    LogInternal(LogDefaults::LogLevel, tmp, LogCategories::Script, uid);
  }
}
template<typename ...Args>
inline void Component::LogTrace(String text, Args... args) const
{
  if (sizeof...(Args) == 0)
    LogInternal(LogDefaults::LogLevel, text, LogCategories::Trace, uid);
  else
  {
    MutableString128 tmp; tmp.format(text, args...);
    LogInternal(LogDefaults::LogLevel, tmp, LogCategories::Trace, uid);
  }
}

template<typename T>
inline SharedPtr<T> component_cast(ComponentRef pComponent)
{
  if (!pComponent)
    return nullptr;
  const ComponentDesc *pDesc = pComponent->pType;
  while (pDesc)
  {
    if (pDesc->id.eq(T::descriptor.id))
      return shared_pointer_cast<T>(pComponent);
    pDesc = pDesc->pSuperDesc;
  }
  return nullptr;
}

} // namespace ep


// HACK: this is here because forward referencing!
// SharedPtr<Component> (and derived types)
template<typename T, typename std::enable_if<std::is_base_of<Component, T>::value>::type* = nullptr> // O_O
inline void epFromVariant(const Variant &v, SharedPtr<T> *pR)
{
  *pR = component_cast<T>(v.asComponent());
}


ptrdiff_t epStringify(Slice<char> buffer, String format, const Component *pComponent, const epVarArg *pArgs);


// HACK: this here because forward referencing! >_<
template<typename... Args>
template <typename X>
void epEvent<Args...>::Subscribe(Component *pC, void(X::*func)(Args...))
{
  pC->subscriber.Subscribe(*this, Delegate((X*)pC, func));
}
template<typename... Args>
template <typename X>
void epEvent<Args...>::Unsubscribe(Component *pC, void(X::*func)(Args...))
{
  pC->subscriber.Unsubscribe(*this, Delegate((X*)pC, func));
}

#endif // EPCOMPONENT_H
