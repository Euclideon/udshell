
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
