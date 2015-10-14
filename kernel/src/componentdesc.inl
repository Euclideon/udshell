
namespace ep
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
inline Variant CGetter::shimFunc(const Getter * const _pGetter, const Component *pThis)
{
  CGetter *pGetter = (CGetter*)_pGetter;
  auto m = pGetter->m;
  m.SetThis((void*)pThis);

  FastDelegate0<T> d;
  d.SetMemento(m);

  return Variant(d());
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
inline void CSetter::shimFunc(const Setter * const _pSetter, Component *pThis, const Variant &value)
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
epforceinline Variant CMethod::Partial<Ret, Args...>::callFuncHack(Slice<Variant> args, FastDelegate<Ret(Args...)> d, internal::Sequence<S...>)
{
  return Variant(d(args[S].as<typename std::remove_const<typename std::remove_reference<Args>::type>::type>()...));
}

template<typename Ret, typename... Args>
inline Variant CMethod::Partial<Ret, Args ...>::shimFunc(const Method * const _pMethod, Component *pThis, Slice<Variant> value)
{
  CMethod *pMethod = (CMethod*)_pMethod;

  auto m = pMethod->m;
  m.SetThis((void*)pThis);

  FastDelegate<Ret(Args...)> d;
  d.SetMemento(m);

  return Variant(callFuncHack(value, d, typename internal::GenSequence<sizeof...(Args)>::type()));
}

template<typename... Args>
struct CMethod::Partial<void, Args...>
{
  template<size_t ...S>
  epforceinline static void callFuncHack(Slice<Variant> args, FastDelegate<void(Args...)> d, internal::Sequence<S...>)
  {
    d(args[S].as<typename std::remove_const<typename std::remove_reference<Args>::type>::type>()...);
  }

  inline static Variant shimFunc(const Method * const _pMethod, Component *pThis, Slice<Variant> value)
  {
    CMethod *pMethod = (CMethod*)_pMethod;

    auto m = pMethod->m;
    m.SetThis((void*)pThis);

    FastDelegate<void(Args...)> d;
    d.SetMemento(m);

    callFuncHack(value, d, typename internal::GenSequence<sizeof...(Args)>::type());
    return Variant();
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
epforceinline Variant CStaticFunc::Partial<Ret, Args...>::callFuncHack(Slice<Variant> args, Ret(*f)(Args...), internal::Sequence<S...>)
{
  return Variant(f(args[S].as<typename std::remove_const<typename std::remove_reference<Args>::type>::type>()...));
}

template<typename Ret, typename... Args>
inline Variant CStaticFunc::Partial<Ret, Args ...>::shimFunc(const StaticFunc * const _pStaticFunc, Slice<Variant> value)
{
  CStaticFunc *pStaticFunc = (CStaticFunc*)_pStaticFunc;

  auto f = (Ret(*)(Args ...)) pStaticFunc->f;

  return Variant(callFuncHack(value, f, typename internal::GenSequence<sizeof...(Args)>::type()));
}

template<typename... Args>
struct CStaticFunc::Partial < void, Args... >
{
  template<size_t ...S>
  epforceinline static void callFuncHack(Slice<Variant> args, void(*f)(Args...), internal::Sequence<S...>)
  {
    f(args[S].as<typename std::remove_const<typename std::remove_reference<Args>::type>::type>()...);
  }

  inline static Variant shimFunc(const StaticFunc * const _staticFunc, Slice<Variant> value)
  {
    CStaticFunc *pStaticFunc = (CStaticFunc*)_pStaticFunc;

    auto f = (void(*)(Args ...)) pStaticFunc->f;

    callFuncHack(value, f, typename internal::GenSequence<sizeof...(Args)>::type());
    return Variant();
  }
};

// event stuff
template<typename X, typename... Args>
inline CEvent::CEvent(epEvent<Args...> X::*ev)
  : VarEvent(nullptr)
{
  pSubscribe = &doSubscribe<X, Args...>;
  pEvent = (void* CEvent::*)ev;
}

template<typename X, typename... Args>
inline void CEvent::doSubscribe(const VarEvent *_pEv, const ComponentRef &c, const Variant::VarDelegate &d)
{
  CEvent *pEv = (CEvent*)_pEv;

  // cast the pointer-to-member back to it's real type
  epEvent<Args...> X::*ev = (epEvent<Args...> X::*)pEv->pEvent;

  // TODO: validate that 'X' is actually a component?
  X *pComponent = (X*)c.ptr();

  // deref the pointer-to-member to get the event we want to subscribe to
  epEvent<Args...> &e = pComponent->*ev;

  Variant v(d);
  e.Subscribe(v.as<Delegate<void(Args...)>>());
}

}  // namespace ep
