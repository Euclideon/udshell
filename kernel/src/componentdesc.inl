
namespace kernel
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
inline Variant CGetter::shimFunc(const Getter * const _pGetter, const ep::Component *pThis)
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
inline void CSetter::shimFunc(const Setter * const _pSetter, ep::Component *pThis, const Variant &value)
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
  shim = &shimFunc<Ret, Args...>;
}
template <typename X, typename Ret, typename... Args>
inline CMethod::CMethod(Ret(X::*func)(Args...) const)
  : Method(nullptr)
{
  m = fastdelegate::MakeDelegate((X*)nullptr, func).GetMemento();
  shim = &shimFunc<Ret, Args...>;
}

template<typename Ret, typename... Args>
inline Variant CMethod::shimFunc(const Method * const _pMethod, ep::Component *pThis, Slice<const Variant> args)
{
  CMethod *pMethod = (CMethod*)_pMethod;

  // TODO: should this be an assert? perhaps soft-error and return a void variant?
  EPASSERT(args.length >= sizeof...(Args), "Not enough args!");

  auto m = pMethod->m;
  m.SetThis((void*)pThis);

  FastDelegate<Ret(Args...)> d;
  d.SetMemento(m);

  return VarCall(d, args);
}

// static function stuff
template <typename Ret, typename... Args>
inline CStaticFunc::CStaticFunc(Ret(*func)(Args...))
  : StaticFunc(nullptr)
{
  f = func;
  shim = &shimFunc<Ret, Args...>;
}

template<typename Ret, typename... Args>
inline Variant CStaticFunc::shimFunc(const StaticFunc * const _pStaticFunc, Slice<const Variant> args)
{
  CStaticFunc *pStaticFunc = (CStaticFunc*)_pStaticFunc;

  // TODO: should this be an assert? perhaps soft-error and return a void variant?
  EPASSERT(args.length >= sizeof...(Args), "Not enough args!");

  return VarCall((Ret(*)(Args ...))pStaticFunc->f, args);
}

// event stuff
template<typename X, typename... Args>
inline CEvent::CEvent(Event<Args...> X::*ev)
  : VarEvent(nullptr)
{
  pSubscribe = &doSubscribe<X, Args...>;
  pEvent = (void* CEvent::*)ev;
}

template<typename X, typename... Args>
inline void CEvent::doSubscribe(const VarEvent *_pEv, const ep::ComponentRef &c, const Variant::VarDelegate &d)
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

}  // namespace kernel
