
namespace ud
{

using fastdelegate::FastDelegate0;
using fastdelegate::FastDelegate1;

// getter stuff
inline Getter::Getter(nullptr_t)
  : shim(nullptr)
{
}
template <class X, class Type>
inline Getter::Getter(Type(X::*func)() const)
{
  m = fastdelegate::MakeDelegate((X*)nullptr, func).GetMemento();
  shim = &shimFunc<Type>;
}

inline udVariant Getter::get(const ud::Component *pThis) const
{
  return shim(this, pThis);
}

template<typename T>
inline udVariant Getter::shimFunc(const Getter * const pGetter, const ud::Component *pThis)
{
  auto m = pGetter->m;
  m.SetThis((void*)pThis);

  FastDelegate0<T> d;
  d.SetMemento(m);

  return udVariant(d());
}

// setter stuff
inline Setter::Setter(nullptr_t)
  : shim(nullptr)
{
}
template <class X, class Type>
inline Setter::Setter(void(X::*func)(Type))
{
  m = fastdelegate::MakeDelegate((X*)nullptr, func).GetMemento();
  shim = &shimFunc<Type>;
}

inline void Setter::set(ud::Component *pThis, const udVariant &value) const
{
  shim(this, pThis, value);
}

template<typename T>
inline void Setter::shimFunc(const Setter * const pSetter, ud::Component *pThis, const udVariant &value)
{
  auto m = pSetter->m;
  m.SetThis((void*)pThis);

  FastDelegate1<T> d;
  d.SetMemento(m);

  d(value.as<typename std::remove_reference<T>::type>());
}

// method stuff
inline Method::Method(nullptr_t)
: shim(nullptr)
{
}
template <typename X, typename Ret, typename... Args>
inline Method::Method(Ret(X::*func)(Args...))
{
  m = fastdelegate::MakeDelegate((X*)nullptr, func).GetMemento();
  shim = &Partial<Ret, Args...>::shimFunc;
}
template <typename X, typename Ret, typename... Args>
inline Method::Method(Ret(X::*func)(Args...) const)
{
  m = fastdelegate::MakeDelegate((X*)nullptr, func).GetMemento();
  shim = &Partial<Ret, Args...>::shimFunc;
}

inline udVariant Method::call(ud::Component *pThis, udSlice<udVariant> args) const
{
  return shim(this, pThis, args);
}

template<typename Ret, typename... Args>
template<size_t ...S>
UDFORCE_INLINE udVariant Method::Partial<Ret, Args...>::callFuncHack(udSlice<udVariant> args, FastDelegate<Ret(Args...)> d, Sequence<S...>)
{
  return udVariant(d(args[S].as<typename std::remove_reference<Args>::type>()...));
}

template<typename Ret, typename... Args>
inline udVariant Method::Partial<Ret, Args ...>::shimFunc(const Method * const pSetter, Component *pThis, udSlice<udVariant> value)
{
  auto m = pSetter->m;
  m.SetThis((void*)pThis);

  FastDelegate<Ret(Args...)> d;
  d.SetMemento(m);

  return udVariant(callFuncHack(value, d, typename GenSequence<sizeof...(Args)>::type()));
}

template<typename... Args>
struct Method::Partial<void, Args...>
{
  template<size_t ...S>
  UDFORCE_INLINE static void callFuncHack(udSlice<udVariant> args, FastDelegate<void(Args...)> d, Sequence<S...>)
  {
    d(args[S].as<typename std::remove_reference<Args>::type>()...);
  }

  inline static udVariant shimFunc(const Method * const pSetter, Component *pThis, udSlice<udVariant> value)
  {
    auto m = pSetter->m;
    m.SetThis((void*)pThis);

    FastDelegate<void(Args...)> d;
    d.SetMemento(m);

    callFuncHack(value, d, typename GenSequence<sizeof...(Args)>::type());
    return udVariant();
  }
};

}  // namespace ud
