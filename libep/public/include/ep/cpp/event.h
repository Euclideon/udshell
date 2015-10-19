#pragma once
#if !defined(_EPEVENT_HPP)
#define _EPEVENT_HPP

#include "ep/cpp/platform.h"
#include "ep/cpp/delegate.h"

using fastdelegate::FastDelegate;

namespace ep {

SHARED_CLASS(Component);
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
  Array<Subscription, 4> subscribers;
};

template<typename... Args>
class Event : public BaseEvent
{
public:
  typedef Delegate<void(Args...)> EvDelegate;

  // subscribe
  void Subscribe(EvDelegate callback)
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
  void Unsubscribe(EvDelegate callback)
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


class Subscriber
{
public:
  ~Subscriber();

  template<typename... Args>
  void Subscribe(Event<Args...> &ev, typename Event<Args...>::EvDelegate d);

  template<typename... Args>
  void Unsubscribe(Event<Args...> &ev, typename Event<Args...>::EvDelegate d);

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

  Array<Subscription, 4> events;
};



/*** implementation ***/

inline BaseEvent::~BaseEvent()
{
  for (auto &s : subscribers)
    s.pSubscriber->RemoveSubscription(s.spM, this);
}

inline void BaseEvent::AddSubscription(const DelegateMementoRef &spM, Subscriber *pSubscriber)
{
  subscribers.pushBack(Subscription(spM, pSubscriber));
}

inline void BaseEvent::RemoveSubscription(const DelegateMementoRef &spM, Subscriber *pSubscriber)
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


inline Subscriber::~Subscriber()
{
  for (auto &e : events)
    e.pEvent->RemoveSubscription(e.spM, this);
}

template<typename... Args>
inline void Subscriber::Subscribe(Event<Args...> &ev, typename Event<Args...>::EvDelegate d)
{
  DelegateMementoRef spM = d.GetMemento();
  ev.AddSubscription(spM, this);
  events.pushBack(Subscription(spM, &ev));
}

template<typename... Args>
inline void Subscriber::Unsubscribe(Event<Args...> &ev, typename Event<Args...>::EvDelegate d)
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

inline void Subscriber::RemoveSubscription(const DelegateMementoRef &spM, BaseEvent *pEvent)
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

} // namespace ep

#endif // _EPEVENT_HPP
