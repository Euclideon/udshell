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

  void AddSubscription(DelegateMemento d, udSubscriber *pSubscriber = nullptr);
  void RemoveSubscription(DelegateMemento d, udSubscriber *pSubscriber = nullptr);

  struct Subscription
  {
    Subscription(DelegateMemento d, udSubscriber *pSubscriber = nullptr)
      : d(d), pSubscriber(pSubscriber) {}

    DelegateMemento d;
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
  template <typename X, typename Y, typename R, typename... Args>
  void Unsubscribe(Y* x, R(X::*func)(Args...))
  {
    Unsubscribe(Delegate(x, func));
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
      d.SetMemento(s.d);
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
  void RemoveSubscription(DelegateMemento d, udBaseEvent *pEvent);

private:
  friend class udBaseEvent;

  struct Subscription
  {
    Subscription(DelegateMemento d, udBaseEvent *pEvent)
      : d(d), pEvent(pEvent) {}

    DelegateMemento d;
    udBaseEvent *pEvent;
  };

  udFixedSlice<Subscription, 3> events;
};


#include "udEvent.inl"

#endif // _UDEVENT_H
