
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
  shim = &Partial<Ret>::shimFunc<Args...>;
}
template <typename X, typename Ret, typename... Args>
inline udMethod::udMethod(Ret(X::*func)(Args...) const)
{
  m = MakeDelegate((X*)nullptr, func).GetMemento();
  shim = &Partial<Ret>::shimFunc<Args...>;
}

inline udVariant udMethod::call(udComponent *pThis, udSlice<udVariant> args) const
{
  return shim(this, pThis, args);
}

template<typename Ret>
template<typename... Args>
inline udVariant udMethod::Partial<Ret>::shimFunc(const udMethod * const pSetter, udComponent *pThis, udSlice<udVariant> value)
{
  auto m = pSetter->m;
  m.SetThis((void*)pThis);

  FastDelegate<Ret(Args...)> d;
  d.SetMemento(m);

  size_t i = 0;
  return udVariant(d(value[i++].as<Args>()...));
}

template<>
struct udMethod::Partial<void>
{
  template<typename... Args>
  inline static udVariant shimFunc(const udMethod * const pSetter, udComponent *pThis, udSlice<udVariant> value)
  {
    auto m = pSetter->m;
    m.SetThis((void*)pThis);

    FastDelegate<void(Args...)> d;
    d.SetMemento(m);

    size_t i = 0;
    d(value[i++].as<Args>()...);
    return udVariant();
  }
};
