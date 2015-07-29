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
  static ComponentDesc descriptor; \
  typedef udSharedPtr<Name> Ref;

namespace ud
{

class Component : public udRefCounted
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

  void SetProperty(udString property, const udVariant &value);
  udVariant GetProperty(udString property) const;

  void SignalPropertyChanged(const PropertyDesc *pProp) { propertyChange[pProp->index].Signal(); }

  udResult SendMessage(udString target, udString message, const udVariant &data);
  udResult SendMessage(Component *pComponent, udString message, const udVariant &data) { return SendMessage(pComponent->uid, message, data); }

  // properties
  udString GetUid() const { return uid; }
  udString GetType() const { return pType->id; }
  udString GetDisplayName() const { return pType->displayName; }
  udString GetDescription() const { return pType->description; }

protected:
  Component(const ComponentDesc *_pType, Kernel *_pKernel, udRCString _uid, InitParams initParams)
    : pType(_pType), pKernel(_pKernel), uid(_uid) {}
  virtual ~Component();

  virtual udResult ReceiveMessage(udString message, udString sender, const udVariant &data);

  size_t NumProperties() const
  {
    return pType->pPropertyTree->Size();
  }
  ptrdiff_t PropertyIndex(udString property) const
  {
    ComponentDesc::PropertyNode *pN = pType->pPropertyTree->Get(property);
    return pN ? pN->index : -1;
  }

  template<typename... Args>
  friend class ::udEvent;

  udSubscriber subscriber;

  udSlice<udEvent<>> propertyChange;

private:
  void Init(InitParams initParams);

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
