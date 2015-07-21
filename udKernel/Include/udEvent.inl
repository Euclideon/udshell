
inline udBaseEvent::~udBaseEvent()
{
  for (auto &s : subscribers)
    s.pSubscriber->RemoveSubscription(s.d, this);
}

inline void udBaseEvent::AddSubscription(DelegateMemento d, udSubscriber *pSubscriber)
{
  subscribers.pushBack(Subscription(d, pSubscriber));
}

inline void udBaseEvent::RemoveSubscription(DelegateMemento d, udSubscriber *pSubscriber)
{
  for (size_t i = 0; i < subscribers.length; ++i)
  {
    if (subscribers[i].d == d && subscribers[i].pSubscriber == pSubscriber)
    {
      subscribers.removeSwapLast(i);
      return;
    }
  }
}


inline udSubscriber::~udSubscriber()
{
  for (auto &e : events)
    e.pEvent->RemoveSubscription(e.d, this);
}

template<typename... Args>
inline void udSubscriber::Subscribe(udEvent<Args...> &ev, typename udEvent<Args...>::Delegate d)
{
  DelegateMemento m = d.GetMemento();
  ev.AddSubscription(m, this);
  events.pushBack(Subscription(m, &ev));
}

template<typename... Args>
inline void udSubscriber::Unsubscribe(udEvent<Args...> &ev, typename udEvent<Args...>::Delegate d)
{
  for (size_t i = 0; i < events.length; ++i)
  {
    if (events[i].d == d && events[i].pEvent == &ev)
    {
      ev.RemoveSubscription(d, this);
      events.removeSwapLast(i);
      return;
    }
  }
}

inline void udSubscriber::RemoveSubscription(DelegateMemento d, udBaseEvent *pEvent)
{
  for (size_t i = 0; i < events.length; ++i)
  {
    if (events[i].d == d && events[i].pEvent == pEvent)
    {
      events.removeSwapLast(i);
      return;
    }
  }
}
