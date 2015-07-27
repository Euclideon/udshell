
// getter stuff
inline udGetter::udGetter(nullptr_t)
  : shim(nullptr)
{
}
template <class X, class Type>
inline udGetter::udGetter(Type(X::*func)() const)
{
  m = MakeDelegate((X*)nullptr, func).GetMemento();
  shim = &shimFunc<Type>;
}

inline udVariant udGetter::get(const udComponent *pThis) const
{
  return shim(this, pThis);
}

template<typename T>
inline udVariant udGetter::shimFunc(const udGetter * const pGetter, const udComponent *pThis)
{
  auto m = pGetter->m;
  m.SetThis((void*)pThis);

  FastDelegate0<T> d;
  d.SetMemento(m);

  return udVariant(d());
}

// setter stuff
inline udSetter::udSetter(nullptr_t)
  : shim(nullptr)
{
}
template <class X, class Type>
inline udSetter::udSetter(void(X::*func)(Type))
{
  m = MakeDelegate((X*)nullptr, func).GetMemento();
  shim = &shimFunc<Type>;
}

inline void udSetter::set(udComponent *pThis, const udVariant &value) const
{
  shim(this, pThis, value);
}

template<typename T>
inline void udSetter::shimFunc(const udSetter * const pSetter, udComponent *pThis, const udVariant &value)
{
  auto m = pSetter->m;
  m.SetThis((void*)pThis);

  FastDelegate1<T> d;
  d.SetMemento(m);

  d(value.as<typename std::remove_reference<T>::type>());
}

// method stuff
inline udMethod::udMethod(nullptr_t)
: shim(nullptr)
{
}
template <typename X, typename Ret, typename... Args>
inline udMethod::udMethod(Ret(X::*func)(Args...))
{
  m = MakeDelegate((X*)nullptr, func).GetMemento();
  shim = &Partial<Ret, Args...>::shimFunc;
}
template <typename X, typename Ret, typename... Args>
inline udMethod::udMethod(Ret(X::*func)(Args...) const)
{
  m = MakeDelegate((X*)nullptr, func).GetMemento();
  shim = &Partial<Ret, Args...>::shimFunc;
}

inline udVariant udMethod::call(udComponent *pThis, udSlice<udVariant> args) const
{
  return shim(this, pThis, args);
}

template<typename Ret, typename... Args>
template<size_t ...S>
UDFORCE_INLINE udVariant udMethod::Partial<Ret, Args...>::callFuncHack(udSlice<udVariant> args, FastDelegate<Ret(Args...)> d, Sequence<S...>)
{
  return udVariant(d(args[S].as<Args>()...));
}

template<typename Ret, typename... Args>
inline udVariant udMethod::Partial<Ret, Args ...>::shimFunc(const udMethod * const pSetter, udComponent *pThis, udSlice<udVariant> value)
{
  auto m = pSetter->m;
  m.SetThis((void*)pThis);

  FastDelegate<Ret(Args...)> d;
  d.SetMemento(m);

  return udVariant(callFuncHack(value, d, typename GenSequence<sizeof...(Args)>::type()));
}

template<typename... Args>
struct udMethod::Partial<void, Args...>
{
  template<size_t ...S>
  UDFORCE_INLINE static void callFuncHack(udSlice<udVariant> args, FastDelegate<void(Args...)> d, Sequence<S...>)
  {
    d(args[S].as<Args>()...);
  }

  inline static udVariant shimFunc(const udMethod * const pSetter, udComponent *pThis, udSlice<udVariant> value)
  {
    auto m = pSetter->m;
    m.SetThis((void*)pThis);

    FastDelegate<void(Args...)> d;
    d.SetMemento(m);

    callFuncHack(value, d, typename GenSequence<sizeof...(Args)>::type());
    return udVariant();
  }
};
