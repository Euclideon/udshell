
namespace kernel {

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
    FastDelegate<void(const RefCounted &, Variant)> d;
    const void **pD = (const void**)&d;
    pD[0] = pThis;
    pD[1] = pSetter;
    d(*data, value);
  }
  else
  {
    // hack to force construct a delegate
    FastDelegate<void(Variant)> d;
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

}  // namespace kernel
