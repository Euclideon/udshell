
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

inline Variant Getter::get(const Component *pThis) const
{
  return shim(this, pThis);
}

template<typename T>
inline Variant Getter::shimFunc(const Getter * const pGetter, const Component *pThis)
{
  auto m = pGetter->m;
  m.SetThis((void*)pThis);

  FastDelegate0<T> d;
  d.SetMemento(m);

  return Variant(d());
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

inline void Setter::set(Component *pThis, const Variant &value) const
{
  shim(this, pThis, value);
}

template<typename T>
inline void Setter::shimFunc(const Setter * const pSetter, Component *pThis, const Variant &value)
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

inline Variant Method::call(Component *pThis, udSlice<Variant> args) const
{
  return shim(this, pThis, args);
}

template<typename Ret, typename... Args>
template<size_t ...S>
UDFORCE_INLINE Variant Method::Partial<Ret, Args...>::callFuncHack(udSlice<Variant> args, FastDelegate<Ret(Args...)> d, Sequence<S...>)
{
  return Variant(d(args[S].as<Args>()...));
}

template<typename Ret, typename... Args>
inline Variant Method::Partial<Ret, Args ...>::shimFunc(const Method * const pSetter, Component *pThis, udSlice<Variant> value)
{
  auto m = pSetter->m;
  m.SetThis((void*)pThis);

  FastDelegate<Ret(Args...)> d;
  d.SetMemento(m);

  return Variant(callFuncHack(value, d, typename GenSequence<sizeof...(Args)>::type()));
}

template<typename... Args>
struct Method::Partial<void, Args...>
{
  template<size_t ...S>
  UDFORCE_INLINE static void callFuncHack(udSlice<Variant> args, FastDelegate<void(Args...)> d, Sequence<S...>)
  {
    d(args[S].as<Args>()...);
  }

  inline static Variant shimFunc(const Method * const pSetter, Component *pThis, udSlice<Variant> value)
  {
    auto m = pSetter->m;
    m.SetThis((void*)pThis);

    FastDelegate<void(Args...)> d;
    d.SetMemento(m);

    callFuncHack(value, d, typename GenSequence<sizeof...(Args)>::type());
    return Variant();
  }
};
}  // namespace ud
