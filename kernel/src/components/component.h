#pragma once
#ifndef EPCOMPONENT_H
#define EPCOMPONENT_H

#include "componentdesc.h"
#include "ep/epevent.h"

#include "udHashMap.h"

#include <stdio.h>

#if defined(SendMessage)
# undef SendMessage
#endif


#define EP_COMPONENT(Name) \
  friend class ep::Kernel; \
  static ep::ComponentDesc descriptor; \
  typedef epSharedPtr<Name> Ref;

namespace ep
{

class Component : public epRefCounted
{
public:
  EP_COMPONENT(Component);

  const ComponentDesc* const pType;
  class Kernel* const pKernel;

  const epSharedString uid;
  epSharedString name;

  bool IsType(epString type) const;
  template<typename T>
  bool IsType() const { return IsType(T::descriptor.id); }

  virtual epVariant GetProperty(epString property) const;
  virtual void SetProperty(epString property, const epVariant &value);

  epVariant CallMethod(epString method, epSlice<epVariant> args);
  template<typename ...Args>
  epVariant CallMethod(epString method, Args... args)
  {
    epVariant varargs[sizeof...(Args) + 1] = { args... };
    return CallMethod(method, epSlice<epVariant>(varargs, sizeof...(Args)));
  }

  void Subscribe(epString eventName, const epVariant::VarDelegate &d);
  void Unsubscribe();

  udResult SendMessage(epString target, epString message, const epVariant &data);
  udResult SendMessage(Component *pComponent, epString message, const epVariant &data) { epMutableString128 temp; temp.concat("@", pComponent->uid); return SendMessage(temp, message, data); }

  const PropertyInfo *GetPropertyInfo(epString name) const
  {
    const PropertyDesc *pDesc = GetPropertyDesc(name);
    return pDesc ? &pDesc->info : nullptr;
  }
  const FunctionInfo *GetMethodInfo(epString name) const
  {
    const MethodDesc *pDesc = GetMethodDesc(name);
    return pDesc ? &pDesc->info : nullptr;
  }
  const EventInfo *GetEventInfo(epString name) const
  {
    const EventDesc *pDesc = GetEventDesc(name);
    return pDesc ? &pDesc->info : nullptr;
  }
  const FunctionInfo *GetStaticFuncInfo(epString name) const
  {
    const StaticFuncDesc *pDesc = GetStaticFuncDesc(name);
    return pDesc ? &pDesc->info : nullptr;
  }

  // built-in component properties
  epString GetUid() const { return uid; }
  epString GetName() const { return name; }
  epString GetType() const { return pType->id; }
  epString GetDisplayName() const { return pType->displayName; }
  epString GetDescription() const { return pType->description; }

  template<typename ...Args> void LogError(epString text, Args... args) const;
  template<typename ...Args> void LogWarning(int level, epString text, Args... args) const;
  template<typename ...Args> void LogDebug(int level, epString text, Args... args) const;
  template<typename ...Args> void LogInfo(int level, epString text, Args... args) const;
  template<typename ...Args> void LogScript(epString text, Args... args) const;
  template<typename ...Args> void LogTrace(epString text, Args... args) const;
  template<typename ...Args> void Log(epString text, Args... args) const; // Calls LogDebug() with level 2

  void SetName(epString name) { this->name = name; }


  void AddDynamicProperty(const PropertyDesc &property);
  void AddDynamicMethod(const MethodDesc &method);
  void AddDynamicEvent(const EventDesc &event);
  void RemoveDynamicProperty(epString name);
  void RemoveDynamicMethod(epString name);
  void RemoveDynamicEvent(epString name);

protected:
  Component(const ComponentDesc *_pType, Kernel *_pKernel, epSharedString _uid, epInitParams initParams)
    : pType(_pType), pKernel(_pKernel), uid(_uid) {}
  virtual ~Component();

  void Init(epInitParams initParams);

  virtual udResult ReceiveMessage(epString message, epString sender, const epVariant &data);

  void LogInternal(int level, epString text, int category, epString componentUID) const;

  // property access
  size_t NumProperties() const { return instanceProperties.Size() + pType->propertyTree.Size(); }
  size_t NumMethods() const { return instanceMethods.Size() + pType->methodTree.Size(); }
  size_t NumEvents() const { return instanceEvents.Size() + pType->eventTree.Size(); }
  size_t NumStaticFuncs() const { return pType->staticFuncTree.Size(); }

  const PropertyDesc *GetPropertyDesc(epString name) const;
  const MethodDesc *GetMethodDesc(epString name) const;
  const EventDesc *GetEventDesc(epString name) const;
  const StaticFuncDesc *GetStaticFuncDesc(epString name) const;

  // TODO: these substantially inflate the size of base Component and are almost always nullptr
  // ...should we move them to a separate allocation?
  epAVLTree<epString, PropertyDesc> instanceProperties;
  epAVLTree<epString, MethodDesc> instanceMethods;
  epAVLTree<epString, EventDesc> instanceEvents;

  epSubscriber subscriber;

private:
  template<typename... Args>
  friend class ::epEvent;
  friend class LuaState;

  Component(const Component &) = delete;    // Still not sold on this
  void operator=(const Component &) = delete;
};

} // namespace ep

#include "components/logger.h"
namespace ep
{

template<typename ...Args>
inline void Component::LogError(epString text, Args... args) const
{
  if (sizeof...(Args) == 0)
    LogInternal(LogDefaults::LogLevel, text, LogCategories::Error, uid);
  else
  {
    epMutableString128 tmp; tmp.format(text, args...);
    LogInternal(LogDefaults::LogLevel, tmp, LogCategories::Error, uid);
  }
}
template<typename ...Args>
inline void Component::LogWarning(int level, epString text, Args... args) const
{
  if (sizeof...(Args) == 0)
    LogInternal(level, text, LogCategories::Warning, uid);
  else
  {
    epMutableString128 tmp; tmp.format(text, args...);
    LogInternal(level, tmp, LogCategories::Warning, uid);
  }
}
template<typename ...Args>
inline void Component::LogDebug(int level, epString text, Args... args) const
{
  if (sizeof...(Args) == 0)
    LogInternal(level, text, LogCategories::Debug, uid);
  else
  {
    epMutableString128 tmp; tmp.format(text, args...);
    LogInternal(level, tmp, LogCategories::Debug, uid);
  }
}
template<typename ...Args>
inline void Component::LogInfo(int level, epString text, Args... args) const
{
  if (sizeof...(Args) == 0)
    LogInternal(level, text, LogCategories::Info, uid);
  else
  {
    epMutableString128 tmp; tmp.format(text, args...);
    LogInternal(level, tmp, LogCategories::Info, uid);
  }
}
template<typename ...Args>
inline void Component::LogScript(epString text, Args... args) const
{
  if (sizeof...(Args) == 0)
    LogInternal(LogDefaults::LogLevel, text, LogCategories::Script, uid);
  else
  {
    epMutableString128 tmp; tmp.format(text, args...);
    LogInternal(LogDefaults::LogLevel, tmp, LogCategories::Script, uid);
  }
}
template<typename ...Args>
inline void Component::LogTrace(epString text, Args... args) const
{
  if (sizeof...(Args) == 0)
    LogInternal(LogDefaults::LogLevel, text, LogCategories::Trace, uid);
  else
  {
    epMutableString128 tmp; tmp.format(text, args...);
    LogInternal(LogDefaults::LogLevel, tmp, LogCategories::Trace, uid);
  }
}
template<typename ...Args>
inline void Component::Log(epString text, Args... args) const
{
  if (sizeof...(Args) == 0)
    LogInternal(LogDefaults::LogLevel, text, LogCategories::Debug, uid);
  else
  {
    epMutableString128 tmp; tmp.format(text, args...);
    LogInternal(LogDefaults::LogLevel, tmp, LogCategories::Debug, uid);
  }
}


template<typename T>
inline epSharedPtr<T> component_cast(ComponentRef pComponent)
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
// epSharedPtr<Component> (and derived types)
template<typename T, typename std::enable_if<std::is_base_of<ep::Component, T>::value>::type* = nullptr> // O_O
inline void epFromVariant(const epVariant &v, epSharedPtr<T> *pR)
{
  *pR = ep::component_cast<T>(v.asComponent());
}


ptrdiff_t epStringify(epSlice<char> buffer, epString format, ep::ComponentRef spComponent, const epVarArg *pArgs);


// HACK: this here because forward referencing! >_<
template<typename... Args>
template <typename X>
void epEvent<Args...>::Subscribe(ep::Component *pC, void(X::*func)(Args...))
{
  pC->subscriber.Subscribe(*this, Delegate((X*)pC, func));
}
template<typename... Args>
template <typename X>
void epEvent<Args...>::Unsubscribe(ep::Component *pC, void(X::*func)(Args...))
{
  pC->subscriber.Unsubscribe(*this, Delegate((X*)pC, func));
}

#endif // EPCOMPONENT_H
