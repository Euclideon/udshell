namespace ep {

// interface for getters, setters, methods, events

// getter glue
class GetterShim
{
public:
  GetterShim(void *pGetter, SharedPtr<const RefCounted> data = nullptr) : pGetter(pGetter), data(data) {}

  explicit operator bool() const { return pGetter != nullptr; }

  Variant get(const Component *pThis) const;

protected:
  void *pGetter;
  SharedPtr<const RefCounted> data;
};

// setter glue
class SetterShim
{
public:
  SetterShim(void *pSetter, SharedPtr<const RefCounted> data = nullptr) : pSetter(pSetter), data(data) {}

  explicit operator bool() const { return pSetter != nullptr; }

  void set(Component *pThis, const Variant &value) const;

protected:
  void *pSetter;
  SharedPtr<const RefCounted> data;
};

// method glue
class MethodShim
{
public:
  MethodShim(void *pMethod, SharedPtr<const RefCounted> data = nullptr) : pMethod(pMethod), data(data) {}

  Variant call(Component *pThis, Slice<const Variant> args) const;

protected:
  void *pMethod;
  SharedPtr<const RefCounted> data;
};

// static funcion glue
class StaticFuncShim
{
public:
  StaticFuncShim(void *pFunc, SharedPtr<const RefCounted> data = nullptr) : pFunc(pFunc), data(data) {}

  Variant call(Slice<const Variant> args) const;

protected:
  void *pFunc;
  SharedPtr<const RefCounted> data;
};

// event glue
class EventShim
{
public:
  EventShim(void *pSubscribe, SharedPtr<const RefCounted> data = nullptr) : pSubscribe(pSubscribe), data(data) {}

  void subscribe(Component *pThis, const Variant::VarDelegate &d) const;

protected:
  void *pSubscribe;
  SharedPtr<const RefCounted> data;
};


// functions
inline Variant GetterShim::get(const ep::Component *pThis) const
{
  // hack to force construct a delegate
  if (data)
  {
    // indirect call with metadata
    FastDelegate<Variant(const RefCounted &)> d;
    const void **pD = (const void**)&d;
    pD[0] = pThis;
    pD[1] = pGetter;
    return d(*data);
  }
  else
  {
    // direct call
    FastDelegate<Variant()> d;
    const void **pD = (const void**)&d;
    pD[0] = pThis;
    pD[1] = pGetter;
    return d();
  }
}

inline void SetterShim::set(ep::Component *pThis, const Variant &value) const
{
  if (data)
  {
    // indirect call with metadata
    FastDelegate<void(const RefCounted &, const Variant &value)> d;
    const void **pD = (const void**)&d;
    pD[0] = pThis;
    pD[1] = pSetter;
    d(*data, value);
  }
  else
  {
    // hack to force construct a delegate
    FastDelegate<void(const Variant &value)> d;
    const void **pD = (const void**)&d;
    pD[0] = pThis;
    pD[1] = pSetter;
    d(value);
  }
}

inline Variant MethodShim::call(ep::Component *pThis, Slice<const Variant> args) const
{
  if (data)
  {
    // indirect call with metadata
    FastDelegate<Variant(const RefCounted &, Slice<const Variant>)> d;
    const void **pD = (const void**)&d;
    pD[0] = pThis;
    pD[1] = pMethod;
    return d(*data, args);
  }
  else
  {
    // hack to force construct a delegate
    FastDelegate<Variant(Slice<const Variant>)> d;
    const void **pD = (const void**)&d;
    pD[0] = pThis;
    pD[1] = pMethod;
    return d(args);
  }
}

inline Variant StaticFuncShim::call(Slice<const Variant> args) const
{
  if (data)
  {
    // indirect call with metadata
    auto pF = (Variant(*)(const RefCounted &, Slice<const Variant>))pFunc;
    return pF(*data, args);
  }
  else
  {
    auto pF = (Variant(*)(Slice<const Variant>))pFunc;
    return pF(args);
  }
}

inline void EventShim::subscribe(ep::Component *pThis, const Variant::VarDelegate &handler) const
{
  if (data)
  {
    // indirect call with metadata
    FastDelegate<void(const RefCounted &, const Variant::VarDelegate&)> d;
    const void **pD = (const void**)&d;
    pD[0] = pThis;
    pD[1] = pSubscribe;
    return d(*data, handler);
  }
  else
  {
    // hack to force construct a delegate
    FastDelegate<void(const Variant::VarDelegate&)> d;
    const void **pD = (const void**)&d;
    pD[0] = pThis;
    pD[1] = pSubscribe;
    return d(handler);
  }
}

} // ep
