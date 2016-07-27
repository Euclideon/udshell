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
public:
  bool hasSubscribers() const;

protected:
  friend class Subscription;

  SubscriptionRef addSubscription(const DelegateMementoRef &spM);
  void removeSubscription(const DelegateMementoRef &spDelegate);
  void removeSubscription(const SubscriptionRef &spSubscription);

  struct Subscriber
  {
    Subscriber(const DelegateMementoRef &spM, const SubscriptionRef spSub)
      : spM(spM), spSubscription(spSub) {}

    DelegateMementoRef spM;
    SubscriptionRef spSubscription;
  };
  Array<Subscriber, 0> subscribers;
};

template<typename... Args>
class Event : public BaseEvent
{
public:
  typedef Delegate<void(Args...)> EvDelegate;

  enum { ParamCount = sizeof...(Args) };

  // subscribe
  SubscriptionRef subscribe(EvDelegate callback)
  {
    return addSubscription(callback.getMemento());
  }
  SubscriptionRef subscribe(const Event<Args...> &ev)
  {
    return subscribe(ev.getDelegate());
  }

  template <class X, class Y>
  SubscriptionRef subscribe(Y *i, void(X::*f)(Args...))        { return subscribe(EvDelegate(i, f)); }
  template <class X, class Y>
  SubscriptionRef subscribe(Y *i, void(X::*f)(Args...) const)  { return subscribe(EvDelegate(i, f)); }

  // unsubscribe
  void unsubscribe(EvDelegate callback)
  {
    removeSubscription(callback.getMemento());
  }
  void unsubscribe(const Event<Args...> &ev)
  {
    unsubscribe(ev.getDelegate());
  }

  template <class X, class Y>
  void unsubscribe(Y *i, void(X::*f)(Args...)) { unsubscribe(EvDelegate(i, f)); }
  template <class X, class Y>
  void unsubscribe(Y *i, void(X::*f)(Args...) const) { unsubscribe(EvDelegate(i, f)); }

  // signal
  void signal(Args... args) const
  {
    for (auto s : subscribers)
    {
      EvDelegate d;
      d.setMemento(s.spM);

      try {
        d(args...);
      } catch (std::exception &e) {
        DebugFormat("Unhandled exception from event handler: {0}\n", e.what());
      } catch (...) {
        DebugFormat("Unhandled C++ exception from event handler!\n");
      }
    }
  }

  // misc
  EvDelegate getDelegate() const
  {
    return EvDelegate(this, &Event::signal);
  }
};


class Subscription : public RefCounted
{
public:
  void unsubscribe();

private:
  template<typename T>
  friend struct SharedPtr;
  friend class BaseEvent;

  Subscription(BaseEvent *pEv)
    : pEv(SafePtr<BaseEvent>(pEv)) {}

  SafePtr<BaseEvent> pEv;
};


/*** implementation ***/

inline void Subscription::unsubscribe()
{
  if (pEv)
  {
    pEv->removeSubscription(SubscriptionRef(this));
    pEv = nullptr;
  }
}

inline SubscriptionRef BaseEvent::addSubscription(const DelegateMementoRef &spM)
{
  for (auto &s : subscribers)
  {
    if (s.spM == spM)
      return s.spSubscription;
  }

  SubscriptionRef spS = SubscriptionRef::create(this);
  subscribers.pushBack(Subscriber(spM, spS));
  return spS;
}

inline void BaseEvent::removeSubscription(const DelegateMementoRef &spDelegate)
{
  for (size_t i = 0; i < subscribers.length; ++i)
  {
    if (subscribers[i].spM->getFastDelegate() == spDelegate->getFastDelegate())
    {
      subscribers.removeSwapLast(i);
      return;
    }
  }
  epDebugWrite("Unsubscribe failed...?\n");
}
inline void BaseEvent::removeSubscription(const SubscriptionRef &spSubscription)
{
  for (size_t i = 0; i < subscribers.length; ++i)
  {
    if (subscribers[i].spSubscription == spSubscription)
    {
      subscribers.removeSwapLast(i);
      return;
    }
  }
  epDebugWrite("Unsubscribe failed...?\n");
}

inline bool BaseEvent::hasSubscribers() const
{
  return !subscribers.empty();
}

} // namespace ep

#endif // _EPEVENT_HPP
