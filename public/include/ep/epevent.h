#pragma once
#if !defined(_UDEVENT_H)
#define _UDEVENT_H

#include "util/uddelegate.h"
#include "util/udslice.h"

using fastdelegate::FastDelegate;

class udSubscriber;
namespace ud
{
  class Component;
  typedef udSharedPtr<Component> ComponentRef;
};

class udBaseEvent
{
public:
  ~udBaseEvent();

protected:
  friend class udSubscriber;

  void AddSubscription(const udDelegateMementoRef &spM, udSubscriber *pSubscriber = nullptr);
  void RemoveSubscription(const udDelegateMementoRef &spM, udSubscriber *pSubscriber = nullptr);

  struct Subscription
  {
    Subscription(const udDelegateMementoRef &spM, udSubscriber *pSubscriber = nullptr)
      : spM(spM), pSubscriber(pSubscriber) {}

    udDelegateMementoRef spM;
    udSubscriber *pSubscriber;
  };
  udFixedSlice<Subscription, 4> subscribers;
};

template<typename... Args>
class udEvent : public udBaseEvent
{
public:
  typedef udDelegate<void(Args...)> Delegate;

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
  void Subscribe(ud::Component *pC, void(X::*func)(Args...));
  template <typename X>
  void Subscribe(ud::ComponentRef c, void(X::*func)(Args...)) { Subscribe(c.ptr(), func); }

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
  void Unsubscribe(ud::Component *pC, void(X::*func)(Args...));
  template <typename X>
  void Unsubscribe(ud::ComponentRef c, void(X::*func)(Args...))
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


class udSubscriber
{
public:
  ~udSubscriber();

  template<typename... Args>
  void Subscribe(udEvent<Args...> &ev, typename udEvent<Args...>::Delegate d);

  template<typename... Args>
  void Unsubscribe(udEvent<Args...> &ev, typename udEvent<Args...>::Delegate d);

protected:
  void RemoveSubscription(const udDelegateMementoRef &spM, udBaseEvent *pEvent);

private:
  friend class udBaseEvent;

  struct Subscription
  {
    Subscription(const udDelegateMementoRef &spM, udBaseEvent *pEvent)
      : spM(spM), pEvent(pEvent) {}

    udDelegateMementoRef spM;
    udBaseEvent *pEvent;
  };

  udFixedSlice<Subscription, 4> events;
};



/*** implementation ***/

inline udBaseEvent::~udBaseEvent()
{
  for (auto &s : subscribers)
    s.pSubscriber->RemoveSubscription(s.spM, this);
}

inline void udBaseEvent::AddSubscription(const udDelegateMementoRef &spM, udSubscriber *pSubscriber)
{
  subscribers.pushBack(Subscription(spM, pSubscriber));
}

inline void udBaseEvent::RemoveSubscription(const udDelegateMementoRef &spM, udSubscriber *pSubscriber)
{
  for (size_t i = 0; i < subscribers.length; ++i)
  {
    if (subscribers[i].spM == spM && subscribers[i].pSubscriber == pSubscriber)
    {
      subscribers.removeSwapLast(i);
      return;
    }
  }
}


inline udSubscriber::~udSubscriber()
{
  for (auto &e : events)
    e.pEvent->RemoveSubscription(e.spM, this);
}

template<typename... Args>
inline void udSubscriber::Subscribe(udEvent<Args...> &ev, typename udEvent<Args...>::Delegate d)
{
  udDelegateMementoRef spM = d.GetMemento();
  ev.AddSubscription(spM, this);
  events.pushBack(Subscription(spM, &ev));
}

template<typename... Args>
inline void udSubscriber::Unsubscribe(udEvent<Args...> &ev, typename udEvent<Args...>::Delegate d)
{
  udDelegateMementoRef spM = d.GetMemento();
  for (size_t i = 0; i < events.length; ++i)
  {
    if (events[i].spM == spM && events[i].pEvent == &ev)
    {
      ev.RemoveSubscription(spM, this);
      events.removeSwapLast(i);
      return;
    }
  }
}

inline void udSubscriber::RemoveSubscription(const udDelegateMementoRef &spM, udBaseEvent *pEvent)
{
  for (size_t i = 0; i < events.length; ++i)
  {
    if (events[i].spM == spM && events[i].pEvent == pEvent)
    {
      events.removeSwapLast(i);
      return;
    }
  }
}

#endif // _UDEVENT_H
