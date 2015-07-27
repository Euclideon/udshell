#pragma once
#if !defined(_UDEVENT_H)
#define _UDEVENT_H

#include "udDelegate.h"
#include "udSlice.h"

using fastdelegate::FastDelegate;

namespace udKernel
{
class Component;
typedef SharedPtr<Component> ComponentRef;
class Subscriber;

class BaseEvent
{
public:
  ~BaseEvent();

protected:
  friend class Subscriber;

  void AddSubscription(const DelegateMementoRef &spM, Subscriber *pSubscriber = nullptr);
  void RemoveSubscription(const DelegateMementoRef &spM, Subscriber *pSubscriber = nullptr);

  struct Subscription
  {
    Subscription(const DelegateMementoRef &spM, Subscriber *pSubscriber = nullptr)
      : spM(spM), pSubscriber(pSubscriber) {}

    DelegateMementoRef spM;
    Subscriber *pSubscriber;
  };
  udFixedSlice<Subscription, 3> subscribers;
};

template<typename... Args>
class Event : public BaseEvent
{
public:
  typedef udDelegate<void(Args...)> Delegate;

  // subscribe
  void Subscribe(Delegate callback)
  {
    AddSubscription(callback.GetMemento());
  }
  void Subscribe(const Event<Args...> &ev)
  {
    Subscribe(ev.GetDelegate());
  }

  template <typename X>
  void Subscribe(Component *pC, void(X::*func)(Args...));
  template <typename X>
  void Subscribe(ComponentRef c, void(X::*func)(Args...)) { Subscribe(c.ptr(), func); }

  // unsubscribe
  void Unsubscribe(Delegate callback)
  {
    RemoveSubscription(callback.GetMemento());
  }
  void Unsubscribe(const Event<Args...> &ev)
  {
    Unsubscribe(ev.GetDelegate());
  }

  template <typename X>
  void Unsubscribe(Component *pC, void(X::*func)(Args...));
  template <typename X>
  void Unsubscribe(ComponentRef c, void(X::*func)(Args...))
  {
    Unsubscribe(c.ptr(), func);
  }

  // signal
  void Signal(Args... args) const
  {
    for (auto s : subscribers)
    {
      Delegate d;
      d.SetMemento(s.spM);
      d(args...);
    }
  }

  // misc
  Delegate GetDelegate() const
  {
    return Delegate(this, &Signal);
  }
};


class Subscriber
{
public:
  ~Subscriber();

  template<typename... Args>
  void Subscribe(Event<Args...> &ev, typename Event<Args...>::Delegate d);

  template<typename... Args>
  void Unsubscribe(Event<Args...> &ev, typename Event<Args...>::Delegate d);

protected:
  void RemoveSubscription(const DelegateMementoRef &spM, BaseEvent *pEvent);

private:
  friend class BaseEvent;

  struct Subscription
  {
    Subscription(const DelegateMementoRef &spM, BaseEvent *pEvent)
      : spM(spM), pEvent(pEvent) {}

    DelegateMementoRef spM;
    BaseEvent *pEvent;
  };

  udFixedSlice<Subscription, 3> events;
};

} // namespace udKernel

#include "udEvent.inl"

#endif // _UDEVENT_H
