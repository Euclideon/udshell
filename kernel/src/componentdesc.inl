
namespace ep
{

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

  FastDelegate<T()> d;
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

  FastDelegate<void(T)> d;
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
inline Variant CMethod::Partial<Ret, Args ...>::shimFunc(const Method * const _pMethod, Component *pThis, Slice<Variant> args)
{
  CMethod *pMethod = (CMethod*)_pMethod;

  // TODO: should this be an assert? perhaps soft-error and return a void variant?
  EPASSERT(args.length >= sizeof...(Args), "Not enough args!");

  auto m = pMethod->m;
  m.SetThis((void*)pThis);

  FastDelegate<Ret(Args...)> d;
  d.SetMemento(m);

  return Variant(callFuncHack(args, d, typename internal::GenSequence<sizeof...(Args)>::type()));
}

template<typename... Args>
struct CMethod::Partial<void, Args...>
{
  template<size_t ...S>
  epforceinline static void callFuncHack(Slice<Variant> args, FastDelegate<void(Args...)> d, internal::Sequence<S...>)
  {
    d(args[S].as<typename std::remove_const<typename std::remove_reference<Args>::type>::type>()...);
  }

  inline static Variant shimFunc(const Method * const _pMethod, Component *pThis, Slice<Variant> args)
  {
    CMethod *pMethod = (CMethod*)_pMethod;

    // TODO: should this be an assert? perhaps soft-error and return a void variant?
    EPASSERT(args.length >= sizeof...(Args), "Not enough args!");

    auto m = pMethod->m;
    m.SetThis((void*)pThis);

    FastDelegate<void(Args...)> d;
    d.SetMemento(m);

    callFuncHack(args, d, typename internal::GenSequence<sizeof...(Args)>::type());
    return Variant(Variant::Type::Void);
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
inline Variant CStaticFunc::Partial<Ret, Args ...>::shimFunc(const StaticFunc * const _pStaticFunc, Slice<Variant> args)
{
  CStaticFunc *pStaticFunc = (CStaticFunc*)_pStaticFunc;

  // TODO: should this be an assert? perhaps soft-error and return a void variant?
  EPASSERT(args.length >= sizeof...(Args), "Not enough args!");

  auto f = (Ret(*)(Args ...)) pStaticFunc->f;

  return Variant(callFuncHack(args, f, typename internal::GenSequence<sizeof...(Args)>::type()));
}

template<typename... Args>
struct CStaticFunc::Partial < void, Args... >
{
  template<size_t ...S>
  epforceinline static void callFuncHack(Slice<Variant> args, void(*f)(Args...), internal::Sequence<S...>)
  {
    f(args[S].as<typename std::remove_const<typename std::remove_reference<Args>::type>::type>()...);
  }

  inline static Variant shimFunc(const StaticFunc * const _pStaticFunc, Slice<Variant> args)
  {
    CStaticFunc *pStaticFunc = (CStaticFunc*)_pStaticFunc;

    // TODO: should this be an assert? perhaps soft-error and return a void variant?
    EPASSERT(args.length >= sizeof...(Args), "Not enough args!");

    auto f = (void(*)(Args ...)) pStaticFunc->f;

    callFuncHack(args, f, typename internal::GenSequence<sizeof...(Args)>::type());
    return Variant(Variant::Type::Void);
  }
};

// event stuff
template<typename X, typename... Args>
inline CEvent::CEvent(Event<Args...> X::*ev)
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
  Event<Args...> X::*ev = (Event<Args...> X::*)pEv->pEvent;

  // TODO: validate that 'X' is actually a component?
  X *pComponent = (X*)c.ptr();

  // deref the pointer-to-member to get the event we want to subscribe to
  Event<Args...> &e = pComponent->*ev;

  Variant v(d);
  e.Subscribe(v.as<Delegate<void(Args...)>>());
}

}  // namespace ep
