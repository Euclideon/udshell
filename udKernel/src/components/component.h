#pragma once
#ifndef UDCOMPONENT_H
#define UDCOMPONENT_H

#include "componentdesc.h"
#include "util/udevent.h"

#include "udHashMap.h"

#include <stdio.h>

#if defined(SendMessage)
# undef SendMessage
#endif


#define UD_COMPONENT(Name) \
  friend class ud::Kernel; \
  static ud::ComponentDesc descriptor; \
  typedef udSharedPtr<Name> Ref;

namespace ud
{

class Component : public udRefCounted
{
public:
  UD_COMPONENT(Component);

  const ComponentDesc* const pType;
  class Kernel* const pKernel;

  const udSharedString uid;
  udSharedString name;

  bool IsType(udString type) const;
  template<typename T>
  bool IsType() const { return IsType(T::descriptor.id); }

  virtual udVariant GetProperty(udString property) const;
  virtual void SetProperty(udString property, const udVariant &value);

  udVariant CallMethod(udString method, udSlice<udVariant> args);
  template<typename ...Args>
  udVariant CallMethod(udString method, Args... args)
  {
    udVariant varargs[sizeof...(Args)] = { args... };
    return CallMethod(method, udSlice<udVariant>(varargs, sizeof...(Args)));
  }

  udResult SendMessage(udString target, udString message, const udVariant &data);
  udResult SendMessage(Component *pComponent, udString message, const udVariant &data) { udMutableString128 temp; temp.concat("@", pComponent->uid); return SendMessage(temp, message, data); }

  const PropertyInfo *GetPropertyInfo(udString name) const
  {
    const PropertyDesc *pDesc = GetPropertyDesc(name);
    return pDesc ? &pDesc->info : nullptr;
  }
  const FunctionInfo *GetMethodInfo(udString name) const
  {
    const MethodDesc *pDesc = GetMethodDesc(name);
    return pDesc ? &pDesc->info : nullptr;
  }
  const EventInfo *GetEventInfo(udString name) const
  {
    const EventDesc *pDesc = GetEventDesc(name);
    return pDesc ? &pDesc->info : nullptr;
  }
  const FunctionInfo *GetStaticFuncInfo(udString name) const
  {
    const StaticFuncDesc *pDesc = GetStaticFuncDesc(name);
    return pDesc ? &pDesc->info : nullptr;
  }

  // built-in component properties
  udString GetUid() const { return uid; }
  udString GetName() const { return name; }
  udString GetType() const { return pType->id; }
  udString GetDisplayName() const { return pType->displayName; }
  udString GetDescription() const { return pType->description; }

  template<typename ...Args> void LogError(udString text, Args... args) const;
  template<typename ...Args> void LogWarning(int level, udString text, Args... args) const;
  template<typename ...Args> void LogDebug(int level, udString text, Args... args) const;
  template<typename ...Args> void LogInfo(int level, udString text, Args... args) const;
  template<typename ...Args> void LogScript(udString text, Args... args) const;
  template<typename ...Args> void LogTrace(udString text, Args... args) const;
  template<typename ...Args> void Log(udString text, Args... args) const; // Calls LogDebug() with level 2

  void SetName(udString name) { this->name = name; }


  void AddDynamicProperty(const PropertyDesc &property);
  void AddDynamicMethod(const MethodDesc &method);
  void AddDynamicEvent(const EventDesc &event);
  void RemoveDynamicProperty(udString name);
  void RemoveDynamicMethod(udString name);
  void RemoveDynamicEvent(udString name);

protected:
  Component(const ComponentDesc *_pType, Kernel *_pKernel, udSharedString _uid, udInitParams initParams)
    : pType(_pType), pKernel(_pKernel), uid(_uid) {}
  virtual ~Component();

  void Init(udInitParams initParams);

  virtual udResult ReceiveMessage(udString message, udString sender, const udVariant &data);

  void LogInternal(int level, udString text, int category, udString componentUID) const;

  // property access
  size_t NumProperties() const { return instanceProperties.Size() + pType->propertyTree.Size(); }
  size_t NumMethods() const { return instanceMethods.Size() + pType->methodTree.Size(); }
  size_t NumEvents() const { return instanceEvents.Size() + pType->eventTree.Size(); }
  size_t NumStaticFuncs() const { return pType->staticFuncTree.Size(); }

  const PropertyDesc *GetPropertyDesc(udString name) const;
  const MethodDesc *GetMethodDesc(udString name) const;
  const EventDesc *GetEventDesc(udString name) const;
  const StaticFuncDesc *GetStaticFuncDesc(udString name) const;

  // TODO: these substantially inflate the size of base Component and are almost always nullptr
  // ...should we move them to a separate allocation?
  udAVLTree<udString, PropertyDesc> instanceProperties;
  udAVLTree<udString, MethodDesc> instanceMethods;
  udAVLTree<udString, EventDesc> instanceEvents;

  udSubscriber subscriber;

private:
  template<typename... Args>
  friend class ::udEvent;
  friend class LuaState;

  Component(const Component &) = delete;    // Still not sold on this
  void operator=(const Component &) = delete;
};

} // namespace ud

#include "components/logger.h"
namespace ud
{

template<typename ...Args>
inline void Component::LogError(udString text, Args... args) const
{
  if (sizeof...(Args) == 0)
    LogInternal(LogDefaults::LogLevel, text, LogCategories::Error, uid);
  else
  {
    udMutableString128 tmp; tmp.format(text, args...);
    LogInternal(LogDefaults::LogLevel, tmp, LogCategories::Error, uid);
  }
}
template<typename ...Args>
inline void Component::LogWarning(int level, udString text, Args... args) const
{
  if (sizeof...(Args) == 0)
    LogInternal(level, text, LogCategories::Warning, uid);
  else
  {
    udMutableString128 tmp; tmp.format(text, args...);
    LogInternal(level, tmp, LogCategories::Warning, uid);
  }
}
template<typename ...Args>
inline void Component::LogDebug(int level, udString text, Args... args) const
{
  if (sizeof...(Args) == 0)
    LogInternal(level, text, LogCategories::Debug, uid);
  else
  {
    udMutableString128 tmp; tmp.format(text, args...);
    LogInternal(level, tmp, LogCategories::Debug, uid);
  }
}
template<typename ...Args>
inline void Component::LogInfo(int level, udString text, Args... args) const
{
  if (sizeof...(Args) == 0)
    LogInternal(level, text, LogCategories::Info, uid);
  else
  {
    udMutableString128 tmp; tmp.format(text, args...);
    LogInternal(level, tmp, LogCategories::Info, uid);
  }
}
template<typename ...Args>
inline void Component::LogScript(udString text, Args... args) const
{
  if (sizeof...(Args) == 0)
    LogInternal(LogDefaults::LogLevel, text, LogCategories::Script, uid);
  else
  {
    udMutableString128 tmp; tmp.format(text, args...);
    LogInternal(LogDefaults::LogLevel, tmp, LogCategories::Script, uid);
  }
}
template<typename ...Args>
inline void Component::LogTrace(udString text, Args... args) const
{
  if (sizeof...(Args) == 0)
    LogInternal(LogDefaults::LogLevel, text, LogCategories::Trace, uid);
  else
  {
    udMutableString128 tmp; tmp.format(text, args...);
    LogInternal(LogDefaults::LogLevel, tmp, LogCategories::Trace, uid);
  }
}
template<typename ...Args>
inline void Component::Log(udString text, Args... args) const
{
  if (sizeof...(Args) == 0)
    LogInternal(LogDefaults::LogLevel, text, LogCategories::Debug, uid);
  else
  {
    udMutableString128 tmp; tmp.format(text, args...);
    LogInternal(LogDefaults::LogLevel, tmp, LogCategories::Debug, uid);
  }
}


template<typename T>
inline udSharedPtr<T> component_cast(ComponentRef pComponent)
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

} // namespace ud


// HACK: this is here because forward referencing!
// udSharedPtr<Component> (and derived types)
template<typename T, typename std::enable_if<std::is_base_of<ud::Component, T>::value>::type* = nullptr> // O_O
inline void udFromVariant(const udVariant &v, udSharedPtr<T> *pR)
{
  *pR = ud::component_cast<T>(v.asComponent());
}


ptrdiff_t udStringify(udSlice<char> buffer, udString format, ud::ComponentRef spComponent, const udVarArg *pArgs);


// HACK: this here because forward referencing! >_<
template<typename... Args>
template <typename X>
void udEvent<Args...>::Subscribe(ud::Component *pC, void(X::*func)(Args...))
{
  pC->subscriber.Subscribe(*this, Delegate((X*)pC, func));
}
template<typename... Args>
template <typename X>
void udEvent<Args...>::Unsubscribe(ud::Component *pC, void(X::*func)(Args...))
{
  pC->subscriber.Unsubscribe(*this, Delegate((X*)pC, func));
}

#endif // UDCOMPONENT_H
