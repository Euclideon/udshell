
// getter stuff
inline udGetter::udGetter(nullptr_t)
{
}
template <class X, class Type>
inline udGetter::udGetter(Type(X::*func)() const)
{
  m = MakeDelegate((X*)nullptr, func).GetMemento();
  shim = &shimFunc<Type>;
}

inline const udVariant udGetter::get(const udComponent *pThis) const
{
  return shim(this, pThis);
}

template<typename T>
static const udVariant udGetter::shimFunc(const udGetter * const pGetter, const udComponent *pThis)
{
  auto m = pGetter->m;
  m.SetThis((void*)pThis);

  FastDelegate0<T> d;
  d.SetMemento(m);

  return udVariant(d());
}

// setter stuff
inline udSetter::udSetter(nullptr_t)
{
}
template <class X, class Type>
inline udSetter::udSetter(void(X::*func)(Type))
{
  m = MakeDelegate((X*)nullptr, func).GetMemento();
  shim = &shimFunc<Type>;
}

inline void udSetter::set(udComponent *pThis, const udVariant value) const
{
  shim(this, pThis, value);
}

template<typename T>
static void udSetter::shimFunc(const udSetter * const pSetter, udComponent *pThis, const udVariant value)
{
  auto m = pSetter->m;
  m.SetThis((void*)pThis);

  FastDelegate1<T> d;
  d.SetMemento(m);

  d(value.as<std::remove_reference<T>::type>());
}
