#pragma once
#if !defined(_EPEVENT_HPP)
#define _EPEVENT_HPP

#include "ep/cpp/platform.h"
#include "ep/cpp/delegate.h"
#include "ep/cpp/safeptr.h"

using fastdelegate::FastDelegate;

namespace ep {

class Subscription;
using SubscriptionRef = SharedPtr<Subscription>;

class BaseEvent : public Safe
{
protected:
  friend class Subscription;

  SubscriptionRef AddSubscription(const DelegateMementoRef &spM);
  void RemoveSubscription(const DelegateMementoRef &spDelegate);
  void RemoveSubscription(const SubscriptionRef &spSubscription);

  struct Subscriber
  {
    Subscriber(const DelegateMementoRef &spM, Subscription *pSubscription)
      : spM(spM), pSubscription(pSubscription) {}

    DelegateMementoRef spM;
    Subscription *pSubscription;
  };
  Array<Subscriber, 0> subscribers;
};

template<typename... Args>
class Event : public BaseEvent
{
public:
  typedef Delegate<void(Args...)> EvDelegate;

  // subscribe
  SubscriptionRef Subscribe(EvDelegate callback)
  {
    return AddSubscription(callback.GetMemento());
  }
  SubscriptionRef Subscribe(const Event<Args...> &ev)
  {
    return Subscribe(ev.GetDelegate());
  }

  template <class X, class Y>
  SubscriptionRef Subscribe(Y *i, void(X::*f)(Args...))        { return Subscribe(EvDelegate(i, f)); }
  template <class X, class Y>
  SubscriptionRef Subscribe(Y *i, void(X::*f)(Args...) const)  { return Subscribe(EvDelegate(i, f)); }
  SubscriptionRef Subscribe(void(*f)(Args...))                 { return Subscribe(EvDelegate(f)); }

  // unsubscribe
  void Unsubscribe(EvDelegate callback)
  {
    RemoveSubscription(callback.GetMemento());
  }
  void Unsubscribe(const Event<Args...> &ev)
  {
    Unsubscribe(ev.GetDelegate());
  }

  template <class X, class Y>
  void Unsubscribe(Y *i, void(X::*f)(Args...)) { Unsubscribe(EvDelegate(i, f)); }
  template <class X, class Y>
  void Unsubscribe(Y *i, void(X::*f)(Args...) const) { Unsubscribe(EvDelegate(i, f)); }
  void Unsubscribe(void(*f)(Args...)) { Unsubscribe(EvDelegate(f)); }

  // signal
  void Signal(Args... args) const
  {
    for (auto s : subscribers)
    {
      EvDelegate d;
      d.SetMemento(s.spM);
      d(args...);
    }
  }

  // misc
  EvDelegate GetDelegate() const
  {
    return EvDelegate(this, &Signal);
  }
};


class Subscription : public RefCounted
{
public:
  void Unsubscribe();

private:
  friend class BaseEvent;

  Subscription(BaseEvent *pEv)
    : pEv(SafePtr<BaseEvent>(pEv)) {}

  SafePtr<BaseEvent> pEv;
};


/*** implementation ***/

inline void Subscription::Unsubscribe()
{
  if (pEv)
  {
    pEv->RemoveSubscription(SubscriptionRef(this));
    pEv = nullptr;
  }
}

inline SubscriptionRef BaseEvent::AddSubscription(const DelegateMementoRef &spM)
{
  for (auto &s : subscribers)
  {
    if (s.spM == spM)
      return SubscriptionRef(s.pSubscription);
  }

  SubscriptionRef spS = SubscriptionRef(new Subscription(this));
  subscribers.pushBack(Subscriber(spM, spS.ptr()));
  return std::move(spS);
}

inline void BaseEvent::RemoveSubscription(const DelegateMementoRef &spDelegate)
{
  for (size_t i = 0; i < subscribers.length; ++i)
  {
    if (subscribers[i].spM->GetFastDelegate() == spDelegate->GetFastDelegate())
    {
      subscribers.removeSwapLast(i);
      return;
    }
  }
  epDebugWrite("Unsubscribe failed...?\n");
}
inline void BaseEvent::RemoveSubscription(const SubscriptionRef &spSubscription)
{
  for (size_t i = 0; i < subscribers.length; ++i)
  {
    if (subscribers[i].pSubscription == spSubscription.ptr())
    {
      subscribers.removeSwapLast(i);
      return;
    }
  }
  epDebugWrite("Unsubscribe failed...?\n");
}


} // namespace ep

#endif // _EPEVENT_HPP
