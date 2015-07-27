#pragma once
#ifndef UDCOMPONENT_H
#define UDCOMPONENT_H

#include "udComponentDesc.h"
#include "udHashMap.h"
#include "udEvent.h"

#include <stdio.h>

#if defined(SendMessage)
# undef SendMessage
#endif


#define UD_COMPONENT(Name) \
  static const ComponentDesc descriptor; \
  typedef SharedPtr<Name> Ref;

namespace udKernel
{

class Component : public RefCounted
{
public:
  friend class Kernel;
  UD_COMPONENT(Component);

  const ComponentDesc* const pType;
  class Kernel* const pKernel;
  const udRCString uid;

  bool IsType(udString type) const;
  template<typename T>
  bool IsType() const { return IsType(T::descriptor.id); }

  const PropertyDesc *FindProperty(udString name) const;

  void SetProperty(udString property, const Variant &value);
  Variant GetProperty(udString property) const;

  udResult SendMessage(udString target, udString message, const Variant &data);
  udResult SendMessage(Component *pComponent, udString message, const Variant &data) { return SendMessage(pComponent->uid, message, data); }

  // properties
  udString GetUid() const { return uid; }
  udString GetType() const { return pType->id; }
  udString GetDisplayName() const { return pType->displayName; }
  udString GetDescription() const { return pType->description; }

protected:
  Component(const ComponentDesc *_pType, Kernel *_pKernel, udRCString _uid, InitParams initParams)
    : pType(_pType), pKernel(_pKernel), uid(_uid) {}
  virtual ~Component() {}

  virtual udResult ReceiveMessage(udString message, udString sender, const Variant &data);

  template<typename... Args>
  friend class Event;
  Subscriber subscriber;

private:
  void Init(InitParams initParams);

  Component(const Component &) = delete;    // Still not sold on this
  void operator=(const Component &) = delete;
};


template<typename T>
inline SharedPtr<T> component_cast(ComponentRef pComponent)
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


// HACK: this is here because forward referencing!
// SharedPtr<Component> (and derived types)
template<typename T, typename std::enable_if<std::is_base_of<Component, T>::value>::type* = nullptr> // O_O
inline void udFromVariant(const Variant &v, SharedPtr<T> *pR)
{
  *pR = component_cast<T>(v.asComponent());
}


// HACK: this here because forward referencing! >_<
template<typename... Args>
template <typename X>
void Event<Args...>::Subscribe(Component *pC, void(X::*func)(Args...))
{
  pC->subscriber.Subscribe(*this, Delegate((X*)pC, func));
}
template<typename... Args>
template <typename X>
void Event<Args...>::Unsubscribe(Component *pC, void(X::*func)(Args...))
{
  pC->subscriber.Unsubscribe(*this, Delegate((X*)pC, func));
}

} // namespace udKernel

#endif // UDCOMPONENT_H
