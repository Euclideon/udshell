
namespace ud
{

using fastdelegate::FastDelegate0;
using fastdelegate::FastDelegate1;

// getter stuff
template <class X, class Type>
inline CGetter::CGetter(Type(X::*func)() const)
  : Getter(nullptr)
{
  m = fastdelegate::MakeDelegate((X*)nullptr, func).GetMemento();
  shim = &shimFunc<Type>;
}

template<typename T>
inline udVariant CGetter::shimFunc(const Getter * const _pGetter, const ud::Component *pThis)
{
  CGetter *pGetter = (CGetter*)_pGetter;
  auto m = pGetter->m;
  m.SetThis((void*)pThis);

  FastDelegate0<T> d;
  d.SetMemento(m);

  return udVariant(d());
}

// setter stuff
template <class X, class Type>
inline CSetter::CSetter(void(X::*func)(Type))
  : Setter(nullptr)
{
  m = fastdelegate::MakeDelegate((X*)nullptr, func).GetMemento();
  shim = &shimFunc<Type>;
}

template<typename T>
inline void CSetter::shimFunc(const Setter * const _pSetter, ud::Component *pThis, const udVariant &value)
{
  CSetter *pSetter = (CSetter*)_pSetter;

  auto m = pSetter->m;
  m.SetThis((void*)pThis);

  FastDelegate1<T> d;
  d.SetMemento(m);

  d(value.as<typename std::remove_reference<T>::type>());
}

// method stuff
template <typename X, typename Ret, typename... Args>
inline CMethod::CMethod(Ret(X::*func)(Args...))
  : Method(nullptr)
{
  m = fastdelegate::MakeDelegate((X*)nullptr, func).GetMemento();
  shim = &Partial<Ret, Args...>::shimFunc;
}
template <typename X, typename Ret, typename... Args>
inline CMethod::CMethod(Ret(X::*func)(Args...) const)
  : Method(nullptr)
{
  m = fastdelegate::MakeDelegate((X*)nullptr, func).GetMemento();
  shim = &Partial<Ret, Args...>::shimFunc;
}

template<typename Ret, typename... Args>
template<size_t ...S>
UDFORCE_INLINE udVariant CMethod::Partial<Ret, Args...>::callFuncHack(udSlice<udVariant> args, FastDelegate<Ret(Args...)> d, Sequence<S...>)
{
  return udVariant(d(args[S].as<typename std::remove_const<typename std::remove_reference<Args>::type>::type>()...));
}

template<typename Ret, typename... Args>
inline udVariant CMethod::Partial<Ret, Args ...>::shimFunc(const Method * const _pMethod, Component *pThis, udSlice<udVariant> value)
{
  CMethod *pMethod = (CMethod*)_pMethod;

  auto m = pMethod->m;
  m.SetThis((void*)pThis);

  FastDelegate<Ret(Args...)> d;
  d.SetMemento(m);

  return udVariant(callFuncHack(value, d, typename GenSequence<sizeof...(Args)>::type()));
}

template<typename... Args>
struct CMethod::Partial<void, Args...>
{
  template<size_t ...S>
  UDFORCE_INLINE static void callFuncHack(udSlice<udVariant> args, FastDelegate<void(Args...)> d, Sequence<S...>)
  {
    d(args[S].as<typename std::remove_const<typename std::remove_reference<Args>::type>::type>()...);
  }

  inline static udVariant shimFunc(const Method * const _pMethod, Component *pThis, udSlice<udVariant> value)
  {
    CMethod *pMethod = (CMethod*)_pMethod;

    auto m = pMethod->m;
    m.SetThis((void*)pThis);

    FastDelegate<void(Args...)> d;
    d.SetMemento(m);

    callFuncHack(value, d, typename GenSequence<sizeof...(Args)>::type());
    return udVariant();
  }
};

// static function stuff
template <typename Ret, typename... Args>
inline CStaticFunc::CStaticFunc(Ret(*func)(Args...))
  : StaticFunc(nullptr)
{
  f = func;
  shim = &Partial<Ret, Args...>::shimFunc;
}

template<typename Ret, typename... Args>
template<size_t ...S>
UDFORCE_INLINE udVariant CStaticFunc::Partial<Ret, Args...>::callFuncHack(udSlice<udVariant> args, Ret (*f)(Args...), Sequence<S...>)
{
  return udVariant(f(args[S].as<typename std::remove_const<typename std::remove_reference<Args>::type>::type>()...));
}

template<typename Ret, typename... Args>
inline udVariant CStaticFunc::Partial<Ret, Args ...>::shimFunc(const StaticFunc * const _pStaticFunc, udSlice<udVariant> value)
{
  CStaticFunc *pStaticFunc = (CStaticFunc*)_pStaticFunc;

  auto f = (Ret(*)(Args ...)) pStaticFunc->f;

  return udVariant(callFuncHack(value, f, typename GenSequence<sizeof...(Args)>::type()));
}

template<typename... Args>
struct CStaticFunc::Partial < void, Args... >
{
  template<size_t ...S>
  UDFORCE_INLINE static void callFuncHack(udSlice<udVariant> args, void (*f)(Args...), Sequence<S...>)
  {
    f(args[S].as<typename std::remove_const<typename std::remove_reference<Args>::type>::type>()...);
  }

  inline static udVariant shimFunc(const StaticFunc * const _staticFunc, udSlice<udVariant> value)
  {
    CStaticFunc *pStaticFunc = (CStaticFunc*)_pStaticFunc;

    auto f = (void(*)(Args ...)) pStaticFunc->f;

    callFuncHack(value, f, typename GenSequence<sizeof...(Args)>::type());
    return udVariant();
  }
};

// event stuff
template<typename X, typename... Args>
inline CEvent::CEvent(udEvent<Args...> X::*ev)
  : VarEvent(nullptr)
{
  pSubscribe = &doSubscribe<X, Args...>;
  pEvent = (void* CEvent::*)ev;
}

template<typename X, typename... Args>
inline void CEvent::doSubscribe(const VarEvent *_pEv, const ComponentRef &c, const udVariant::VarDelegate &d)
{
  CEvent *pEv = (CEvent*)_pEv;

  // cast the pointer-to-member back to it's real type
  udEvent<Args...> X::*ev = (udEvent<Args...> X::*)pEv->pEvent;

  // TODO: validate that 'X' is actually a component?
  X *pComponent = (X*)c.ptr();

  // deref the pointer-to-member to get the event we want to subscribe to
  udEvent<Args...> &e = pComponent->*ev;

  udVariant v(d);
  e.Subscribe(v.as<udDelegate<void(Args...)>>());
}

}  // namespace ud
