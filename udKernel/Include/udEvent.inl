
inline udBaseEvent::~udBaseEvent()
{
  for (auto &s : subscribers)
    s.pSubscriber->RemoveSubscription(s.m, this);
}

inline void udBaseEvent::AddSubscription(DelegateMemento m, udSubscriber *pSubscriber)
{
  subscribers.pushBack(Subscription(m, pSubscriber));
}

inline void udBaseEvent::RemoveSubscription(DelegateMemento m, udSubscriber *pSubscriber)
{
  for (size_t i = 0; i < subscribers.length; ++i)
  {
    if (subscribers[i].m == m && subscribers[i].pSubscriber == pSubscriber)
    {
      subscribers.removeSwapLast(i);
      return;
    }
  }
}


inline udSubscriber::~udSubscriber()
{
  for (auto &e : events)
    e.pEvent->RemoveSubscription(e.m, this);
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
  DelegateMemento m = d.GetMemento();
  for (size_t i = 0; i < events.length; ++i)
  {
    if (events[i].m == m && events[i].pEvent == &ev)
    {
      ev.RemoveSubscription(m, this);
      events.removeSwapLast(i);
      return;
    }
  }
}

inline void udSubscriber::RemoveSubscription(DelegateMemento m, udBaseEvent *pEvent)
{
  for (size_t i = 0; i < events.length; ++i)
  {
    if (events[i].m == m && events[i].pEvent == pEvent)
    {
      events.removeSwapLast(i);
      return;
    }
  }
}
