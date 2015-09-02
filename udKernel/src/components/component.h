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
  friend class Kernel; \
  static ComponentDesc descriptor; \
  typedef udSharedPtr<Name> Ref;

namespace ud
{

class Component : public udRefCounted
{
public:
  UD_COMPONENT(Component);

  const ComponentDesc* const pType;
  class Kernel* const pKernel;

  const udRCString uid;
  udRCString name;

  bool IsType(udString type) const;
  template<typename T>
  bool IsType() const { return IsType(T::descriptor.id); }

  virtual void SetProperty(udString property, const udVariant &value);
  virtual udVariant GetProperty(udString property) const;

  udResult SendMessage(udString target, udString message, const udVariant &data);
  udResult SendMessage(Component *pComponent, udString message, const udVariant &data) { udFixedString128 temp; temp.concat("@", pComponent->uid); return SendMessage(temp , message, data); }

  const PropertyInfo *GetPropertyInfo(udString name) const
  {
    const PropertyDesc *pDesc = GetPropertyDesc(name);
    return pDesc ? &pDesc->info : nullptr;
  }
  const MethodInfo *GetMethodInfo(udString name) const
  {
    const MethodDesc *pDesc = GetMethodDesc(name);
    return pDesc ? &pDesc->info : nullptr;
  }
  const EventInfo *GetEventInfo(udString name) const
  {
    const EventDesc *pDesc = GetEventDesc(name);
    return pDesc ? &pDesc->info : nullptr;
  }

  // built-in component properties
  udString GetUid() const { return uid; }
  udString GetName() const { return name; }
  udString GetType() const { return pType->id; }
  udString GetDisplayName() const { return pType->displayName; }
  udString GetDescription() const { return pType->description; }

  void LogError(int level, udString text) const;
  void LogWarning(int level, udString text) const;
  void LogDebug(int level, udString text) const;
  void LogInfo(int level, udString text) const;
  void LogScript(int level, udString text) const;
  void LogTrace(int level, udString text) const;
  void Log(int level, udString text) const;

  void SetName(udString name) { this->name = name; }

protected:
  Component(const ComponentDesc *_pType, Kernel *_pKernel, udRCString _uid, udInitParams initParams)
    : pType(_pType), pKernel(_pKernel), uid(_uid) {}
  virtual ~Component();

  void Init(udInitParams initParams);

  virtual udResult ReceiveMessage(udString message, udString sender, const udVariant &data);

  // property access
  virtual const PropertyDesc *GetPropertyDesc(udString name) const;
  virtual const MethodDesc *GetMethodDesc(udString name) const;
  virtual const EventDesc *GetEventDesc(udString name) const;

  udSubscriber subscriber;

private:
  template<typename... Args>
  friend class ::udEvent;
  friend class LuaState;

  Component(const Component &) = delete;    // Still not sold on this
  void operator=(const Component &) = delete;
};


template<typename T>
inline udSharedPtr<T> component_cast(ComponentRef pComponent)
{
  if (!pComponent)
    return nullptr;
  const ComponentDesc *pDesc = pComponent->pType;
  while (pDesc)
  {
    if (pDesc->id.eq(T::descriptor.id))
      return static_pointer_cast<T>(pComponent);
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
