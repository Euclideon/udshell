#pragma once
#if !defined(_UDEVENT_H)
#define _UDEVENT_H

#include "3rdparty/FastDelegate.h"
#include "udSharedPtr.h"
#include "udSlice.h"

using fastdelegate::FastDelegate;
using fastdelegate::DelegateMemento;

class udComponent;
typedef udSharedPtr<udComponent> udComponentRef;
class udSubscriber;

class udBaseEvent
{
public:
  ~udBaseEvent();

protected:
  friend class udSubscriber;

  void AddSubscription(DelegateMemento m, udSubscriber *pSubscriber = nullptr);
  void RemoveSubscription(DelegateMemento m, udSubscriber *pSubscriber = nullptr);

  struct Subscription
  {
    Subscription(DelegateMemento m, udSubscriber *pSubscriber = nullptr)
      : m(m), pSubscriber(pSubscriber) {}

    DelegateMemento m;
    udSubscriber *pSubscriber;
  };
  udFixedSlice<Subscription, 3> subscribers;
};

template<typename... Args>
class udEvent : public udBaseEvent
{
public:
  typedef FastDelegate<void(Args...)> Delegate;

  // subscribe
  void Subscribe(Delegate callback)
  {
    AddSubscription(callback.GetMemento());
  }
  void Subscribe(const udEvent<Args...> &ev)
  {
    Subscribe(ev.GetDelegate());
  }

  template <typename X>
  void Subscribe(udComponent *pC, void(X::*func)(Args...));
  template <typename X>
  void Subscribe(udComponentRef c, void(X::*func)(Args...))
  {
    Subscribe(c.ptr(), func);
  }

  // unsubscribe
  void Unsubscribe(Delegate callback)
  {
    RemoveSubscription(callback.GetMemento());
  }
  void Unsubscribe(const udEvent<Args...> &ev)
  {
    Unsubscribe(ev.GetDelegate());
  }

  template <typename X>
  void Unsubscribe(udComponent *pC, void(X::*func)(Args...));
  template <typename X>
  void Unsubscribe(udComponentRef c, void(X::*func)(Args...))
  {
    Unsubscribe(c.ptr(), func);
  }

  // signal
  void Signal(Args... args) const
  {
    for (auto s : subscribers)
    {
      Delegate d;
      d.SetMemento(s.m);
      d(args...);
    }
  }

  // misc
  Delegate GetDelegate() const
  {
    return Delegate(this, &Signal);
  }
};


class udSubscriber
{
public:
  ~udSubscriber();

  template<typename... Args>
  void Subscribe(udEvent<Args...> &ev, typename udEvent<Args...>::Delegate d);

  template<typename... Args>
  void Unsubscribe(udEvent<Args...> &ev, typename udEvent<Args...>::Delegate d);

protected:
  void RemoveSubscription(DelegateMemento m, udBaseEvent *pEvent);

private:
  friend class udBaseEvent;

  struct Subscription
  {
    Subscription(DelegateMemento m, udBaseEvent *pEvent)
      : m(m), pEvent(pEvent) {}

    DelegateMemento m;
    udBaseEvent *pEvent;
  };

  udFixedSlice<Subscription, 3> events;
};


#include "udEvent.inl"

#endif // _UDEVENT_H
