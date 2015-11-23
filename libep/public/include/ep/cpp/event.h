#pragma once
#if !defined(_EPEVENT_HPP)
#define _EPEVENT_HPP

#include "ep/cpp/platform.h"
#include "ep/cpp/delegate.h"

using fastdelegate::FastDelegate;

namespace ep {

class BaseEvent
{
public:
  ~BaseEvent();

protected:
  friend class Subscriber;

  void AddSubscription(const DelegateMementoRef &spM);
//  void RemoveSubscription(const DelegateMementoRef &spM);

  struct Subscription
  {
    Subscription(const DelegateMementoRef &spM)
      : spM(spM) {}

    DelegateMementoRef spM;
  };
  Array<Subscription, 0> subscribers;
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

  template <class X, class Y>
  void Subscribe(Y *i, void(X::*f)(Args...))        { Subscribe(EvDelegate(i, f)); }
  template <class X, class Y>
  void Subscribe(Y *i, void(X::*f)(Args...) const)  { Subscribe(EvDelegate(i, f)); }
  void Subscribe(void(*f)(Args...))                 { Subscribe(EvDelegate(f)); }

//  template <typename X>
//  void Subscribe(Component *pC, void(X::*func)(Args...));
//  template <typename X>
//  void Subscribe(ComponentRef c, void(X::*func)(Args...)) { Subscribe(c.ptr(), func); }

  // unsubscribe
  // TODO: this should rather take a subscription handle
  void Unsubscribe(EvDelegate callback)
  {
//    RemoveSubscription(callback.GetMemento());
  }
  void Unsubscribe(const Event<Args...> &ev)
  {
//    Unsubscribe(ev.GetDelegate());
  }

  template <class X, class Y>
  void Unsubscribe(Y *i, void(X::*f)(Args...))        { Unsubscribe(EvDelegate(i, f)); }
  template <class X, class Y>
  void Unsubscribe(Y *i, void(X::*f)(Args...) const)  { Unsubscribe(EvDelegate(i, f)); }
  void Unsubscribe(void(*f)(Args...))                 { Unsubscribe(EvDelegate(f)); }


//  template <typename X>
//  void Unsubscribe(Component *pC, void(X::*func)(Args...));
//  template <typename X>
//  void Unsubscribe(ComponentRef c, void(X::*func)(Args...))
//  {
//    Unsubscribe(c.ptr(), func);
//  }

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


/*** implementation ***/

inline BaseEvent::~BaseEvent()
{
}

inline void BaseEvent::AddSubscription(const DelegateMementoRef &spM)
{
  subscribers.pushBack(Subscription(spM));
}

} // namespace ep

#endif // _EPEVENT_HPP
