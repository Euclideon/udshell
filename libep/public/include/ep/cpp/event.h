#pragma once
#if !defined(_EPEVENT_HPP)
#define _EPEVENT_HPP

#include "ep/cpp/platform.h"
#include "ep/cpp/delegate.h"

using fastdelegate::FastDelegate;

class epSubscriber;
namespace ep
{
  class Component;
  typedef SharedPtr<Component> ComponentRef;
};

class epBaseEvent
{
public:
  ~epBaseEvent();

protected:
  friend class epSubscriber;

  void AddSubscription(const DelegateMementoRef &spM, epSubscriber *pSubscriber = nullptr);
  void RemoveSubscription(const DelegateMementoRef &spM, epSubscriber *pSubscriber = nullptr);

  struct Subscription
  {
    Subscription(const DelegateMementoRef &spM, epSubscriber *pSubscriber = nullptr)
      : spM(spM), pSubscriber(pSubscriber) {}

    DelegateMementoRef spM;
    epSubscriber *pSubscriber;
  };
  Array<Subscription, 4> subscribers;
};

template<typename... Args>
class epEvent : public epBaseEvent
{
public:
  typedef Delegate<void(Args...)> Delegate;

  // subscribe
  void Subscribe(Delegate callback)
  {
    AddSubscription(callback.GetMemento());
  }
  void Subscribe(const epEvent<Args...> &ev)
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
  void Unsubscribe(const epEvent<Args...> &ev)
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


class epSubscriber
{
public:
  ~epSubscriber();

  template<typename... Args>
  void Subscribe(epEvent<Args...> &ev, typename epEvent<Args...>::Delegate d);

  template<typename... Args>
  void Unsubscribe(epEvent<Args...> &ev, typename epEvent<Args...>::Delegate d);

protected:
  void RemoveSubscription(const DelegateMementoRef &spM, epBaseEvent *pEvent);

private:
  friend class epBaseEvent;

  struct Subscription
  {
    Subscription(const DelegateMementoRef &spM, epBaseEvent *pEvent)
      : spM(spM), pEvent(pEvent) {}

    DelegateMementoRef spM;
    epBaseEvent *pEvent;
  };

  Array<Subscription, 4> events;
};



/*** implementation ***/

inline epBaseEvent::~epBaseEvent()
{
  for (auto &s : subscribers)
    s.pSubscriber->RemoveSubscription(s.spM, this);
}

inline void epBaseEvent::AddSubscription(const DelegateMementoRef &spM, epSubscriber *pSubscriber)
{
  subscribers.pushBack(Subscription(spM, pSubscriber));
}

inline void epBaseEvent::RemoveSubscription(const DelegateMementoRef &spM, epSubscriber *pSubscriber)
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


inline epSubscriber::~epSubscriber()
{
  for (auto &e : events)
    e.pEvent->RemoveSubscription(e.spM, this);
}

template<typename... Args>
inline void epSubscriber::Subscribe(epEvent<Args...> &ev, typename epEvent<Args...>::Delegate d)
{
  DelegateMementoRef spM = d.GetMemento();
  ev.AddSubscription(spM, this);
  events.pushBack(Subscription(spM, &ev));
}

template<typename... Args>
inline void epSubscriber::Unsubscribe(epEvent<Args...> &ev, typename epEvent<Args...>::Delegate d)
{
  DelegateMementoRef spM = d.GetMemento();
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

inline void epSubscriber::RemoveSubscription(const DelegateMementoRef &spM, epBaseEvent *pEvent)
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

#endif // _EPEVENT_HPP
