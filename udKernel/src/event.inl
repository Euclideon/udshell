namespace ud
{

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
inline void Subscriber::Subscribe(Event<Args...> &ev, typename Event<Args...>::Delegate d)
{
  DelegateMementoRef spM = d.GetMemento();
  ev.AddSubscription(spM, this);
  events.pushBack(Subscription(spM, &ev));
}

template<typename... Args>
inline void Subscriber::Unsubscribe(Event<Args...> &ev, typename Event<Args...>::Delegate d)
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
} // namespace ud
