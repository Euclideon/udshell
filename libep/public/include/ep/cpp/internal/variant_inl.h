#include <utility>

#include "ep/cpp/math.h"
#include "ep/cpp/freelist.h"

ptrdiff_t epStringifyVariant(ep::Slice<char> buffer, ep::String format, const ep::Variant &var, const ep::VarArg*);

namespace ep {
namespace internal {

template<> struct StringifyProxy<Variant>
{
  inline static ptrdiff_t stringify(Slice<char> buffer, String format, const void *pData, const VarArg *pArgs) { return epStringifyVariant(buffer, format, *(Variant*)pData, pArgs); }
  inline static int64_t intify(const void *pData) { return ((Variant*)pData)->asInt(); }
};

// these horrid templates generate an integer sequence
template<size_t ...> struct Sequence { };
template<int N, size_t ...S> struct GenSequence : GenSequence<N-1, N-1, S...> { };
template<size_t ...S> struct GenSequence<0, S...> { typedef Sequence<S...> type; };

// variadic function call helpers
template<typename R, typename... Args, size_t... S>
epforceinline R TupleCallHack(R(*f)(Args...), const std::tuple<Args...> &args, ep::internal::Sequence<S...>)
{
  return f(std::get<S>(args)...);
}

template<typename R, typename... Args>
struct VarCallHack
{
  template<size_t... S>
  epforceinline static Variant call(R(*f)(Args...), Slice<const Variant> args, ep::internal::Sequence<S...>)
  {
    try {
      return Variant(f(args[S].as<typename std::remove_const<typename std::remove_reference<Args>::type>::type>()...));
    } catch (EPException &e) {
      return Variant(e.claim());
    } catch (std::exception &e) {
      return Variant(allocError(Result::CppException, e.what()));
    } catch (...) {
      return Variant(allocError(Result::CppException, "C++ exception"));
    }
  }
};
template<typename... Args>
struct VarCallHack<void, Args...>
{
  template<size_t... S>
  epforceinline static Variant call(void(*f)(Args...), Slice<const Variant> args, ep::internal::Sequence<S...>)
  {
    try {
      f(args[S].as<typename std::remove_const<typename std::remove_reference<Args>::type>::type>()...);
      return Variant();
    } catch (EPException &e) {
      return Variant(e.claim());
    } catch (std::exception &e) {
      return Variant(allocError(Result::CppException, e.what()));
    } catch (...) {
      return Variant(allocError(Result::CppException, "C++ exception"));
    }
  }
};

template<typename R, typename... Args>
struct MethodCallHack
{
  template<size_t... S>
  epforceinline static Variant call(FastDelegate<R(Args...)> f, Slice<const Variant> args, ep::internal::Sequence<S...>)
  {
    try {
      return Variant(f(args[S].as<typename std::remove_const<typename std::remove_reference<Args>::type>::type>()...));
    } catch (EPException &e) {
      return Variant(e.claim());
    } catch (std::exception &e) {
      return Variant(allocError(Result::CppException, e.what()));
    } catch (...) {
      return Variant(allocError(Result::CppException, "C++ exception"));
    }
  }
};
template<typename... Args>
struct MethodCallHack<void, Args...>
{
  template<size_t... S>
  epforceinline static Variant call(FastDelegate<void(Args...)> f, Slice<const Variant> args, ep::internal::Sequence<S...>)
  {
    try {
      f(args[S].as<typename std::remove_const<typename std::remove_reference<Args>::type>::type>()...);
      return Variant();
    } catch (EPException &e) {
      return Variant(e.claim());
    } catch (std::exception &e) {
      return Variant(allocError(Result::CppException, e.what()));
    } catch (...) {
      return Variant(allocError(Result::CppException, "C++ exception"));
    }
  }
};

} // namespace internal

epforceinline ptrdiff_t Compare<Variant>::operator()(const Variant &a, const Variant &b)
{
  return a.compare(b);
}

// these perform variadic function calls for different function and arg types
template<typename R, typename... Args>
epforceinline R TupleCall(R(*f)(Args...), const std::tuple<Args...> &args)
{
  return internal::TupleCallHack(f, args, typename internal::GenSequence<sizeof...(Args)>::type());
}

template<typename R, typename... Args>
epforceinline Variant VarCall(R(*f)(Args...), Slice<const Variant> args)
{
  return internal::VarCallHack<R, Args...>::call(f, args, typename internal::GenSequence<sizeof...(Args)>::type());
}

template<typename R, typename... Args>
epforceinline Variant VarCall(FastDelegate<R(Args...)> f, Slice<const Variant> args)
{
  return internal::MethodCallHack<R, Args...>::call(f, args, typename internal::GenSequence<sizeof...(Args)>::type());
}

inline ep::SubscriptionRef VarEvent::AddSubscription(const ep::VarDelegate &del)
{
  return BaseEvent::AddSubscription(del.GetMemento());
}


// *** Variant ***

// constructors...
inline Variant::Variant()
{
  t = (size_t)Type::Void;
  ownsContent = 0;
  length = 0;
}

inline Variant::Variant(Variant &&rval)
{
  t = rval.t;
  ownsContent = rval.ownsContent;
  length = rval.length;
  i = rval.i;

  rval.t = (size_t)Type::Null;
  rval.ownsContent = 0;
}

inline Variant::Variant(const Variant &val)
{
  t = val.t;
  ownsContent = val.ownsContent;
  length = val.length;
  i = val.i;

  if (ownsContent)
    copyContent(val);
}

inline Variant::Variant(Type _t)
{
  EPASSERT(_t == Type::Void, "Only void type assignment supported!");
  t = (size_t)_t;
  ownsContent = 0;
  length = 0;
}
inline Variant::Variant(nullptr_t)
{
  t = (size_t)Type::Null;
  ownsContent = 0;
  length = 0;
}
inline Variant::Variant(ErrorState *pErrorState)
{
  t = (size_t)Type::Error;
  ownsContent = 0;
  length = 0;
  p = pErrorState;
}

inline Variant::Variant(bool _b)
{
  t = (size_t)Type::Bool;
  ownsContent = 0;
  length = 0;
  b = (char)_b;
}
inline Variant::Variant(int64_t _i)
{
  t = (size_t)Type::Int;
  ownsContent = 0;
  length = 0;
  i = _i;
}
inline Variant::Variant(double _f)
{
  t = (size_t)Type::Float;
  ownsContent = 0;
  length = 0;
  f = _f;
}
inline Variant::Variant(size_t val, const EnumDesc *pDesc, bool isBitfield)
{
  t = isBitfield ? (size_t)Type::Bitfield : (size_t)Type::Enum;
  ownsContent = 0;
  length = val;
  p = (void*)pDesc;
}
inline Variant::Variant(SharedPtr<RefCounted> &&spC, SharedPtrType type)
{
  if (spC.ptr())
  {
    t = (size_t)Type::SharedPtr;
    ownsContent = 1;
    length = (size_t)type;
    epConstruct(&sp) SharedPtr<RefCounted>(std::move(spC));
  }
  else
  {
    t = (size_t)Type::Null;
    ownsContent = false;
    length = 0;
    sp = nullptr;
  }
}
inline Variant::Variant(RefCounted *pRef, SharedPtrType type, bool _ownsContent)
{
  if (pRef)
  {
    t = (size_t)Type::SharedPtr;
    ownsContent = _ownsContent ? 1 : 0;
    length = (size_t)type;
    if (ownsContent)
      epConstruct(&sp) SharedPtr<RefCounted>(pRef);
    else
      sp = pRef;
  }
  else
  {
    t = (size_t)Type::Null;
    ownsContent = false;
    length = 0;
    sp = nullptr;
  }
}
inline Variant::Variant(const SharedPtr<RefCounted> &spRC, SharedPtrType type, bool _ownsContent)
  : Variant(spRC.ptr(), type, _ownsContent)
{}
inline Variant::Variant(VarDelegate &&d)
  : Variant(std::move((SharedPtr<RefCounted>&)d.m), SharedPtrType::Delegate)
{}
inline Variant::Variant(const VarDelegate &d)
  : Variant((const SharedPtr<RefCounted>&)d.m, SharedPtrType::Delegate)
{}
inline Variant::Variant(ComponentRef &&spC)
  : Variant(std::move((SharedPtr<RefCounted>&)spC), SharedPtrType::Component)
{}
inline Variant::Variant(const ComponentRef &spC)
  : Variant((RefCounted*)spC.ptr(), SharedPtrType::Component)
{}
inline Variant::Variant(SubscriptionRef &&spS)
  : Variant(std::move((SharedPtr<RefCounted>&)spS), SharedPtrType::Subscription)
{}
inline Variant::Variant(const SubscriptionRef &spS)
  : Variant((const SharedPtr<RefCounted>&)spS, SharedPtrType::Subscription)
{}
inline Variant::Variant(VarMap &&spS)
  : Variant(std::move((SharedPtr<RefCounted>&)spS.ptr), SharedPtrType::AssocArray)
{}
inline Variant::Variant(const VarMap &spS)
  : Variant((const SharedPtr<RefCounted>&)spS.ptr, SharedPtrType::AssocArray)
{}
inline Variant::Variant(VarRange &&spR)
  : Variant(std::move((SharedPtr<RefCounted>&)spR), SharedPtrType::Range)
{}
inline Variant::Variant(const VarRange &spR)
  : Variant((const SharedPtr<RefCounted>&)spR, SharedPtrType::Range)
{}

template<size_t Len>
inline Variant::Variant(const MutableString<Len> &s)
  : Variant((String)s)
{}
template<size_t Len>
inline Variant::Variant(MutableString<Len> &&s)
{
  if (s.hasAllocation() && s.length)
  {
    // if the rvalue has an allocation, we can just claim it
    t = (size_t)Type::String;
    ownsContent = 1;
    length = s.length;
    this->s = s.ptr;
    s.ptr = nullptr;
  }
  else
    epConstruct(this) Variant((String)s);
}
inline Variant::Variant(const SharedString &s)
{
  t = (size_t)Type::String;
  ownsContent = s.ptr ? 1 : 0;
  length = s.length;
  this->s = s.ptr;
  if (s.ptr)
    ++internal::GetSliceHeader(s.ptr)->refCount;
}
inline Variant::Variant(SharedString &&s)
{
  t = (size_t)Type::String;
  ownsContent = s.ptr ? 1 : 0;
  length = s.length;
  this->s = s.ptr;
  if (s.ptr)
    s.ptr = nullptr;
}

template<size_t Len>
inline Variant::Variant(const Array<Variant, Len> &a)
  : Variant(Slice<Variant>(a))
{}
template<size_t Len>
inline Variant::Variant(Array<Variant, Len> &&a)
{
  if (a.hasAllocation() && a.length)
  {
    // if the rvalue has an allocation, we can just claim it
    t = (size_t)Type::Array;
    ownsContent = 1;
    length = a.length;
    this->p = a.ptr;
    a.ptr = nullptr;
  }
  else
    epConstruct(this) Variant((Slice<Variant>)a);
}
inline Variant::Variant(const VarArray &a)
{
  t = (size_t)Type::Array;
  ownsContent = a.ptr ? 1 : 0;
  length = a.length;
  this->p = a.ptr;
  if (a.ptr)
    ++internal::GetSliceHeader(a.ptr)->refCount;
}
inline Variant::Variant(VarArray &&a)
{
  t = (size_t)Type::Array;
  ownsContent = a.ptr ? 1 : 0;
  length = a.length;
  this->p = a.ptr;
  if (a.ptr)
    a.ptr = nullptr;
}

inline Variant::~Variant()
{
  if (ownsContent)
    destroy();
}


inline Variant& Variant::operator=(Variant &&rval)
{
  if (this != &rval)
  {
    this->~Variant();

    t = rval.t;
    ownsContent = rval.ownsContent;
    length = rval.length;
    i = rval.i;

    rval.t = (size_t)Type::Null;
    rval.ownsContent = false;
  }
  return *this;
}

inline Variant& Variant::operator=(const Variant &rval)
{
  if (this != &rval)
  {
    this->~Variant();
    epConstruct(this) Variant(rval);
  }
  return *this;
}

namespace internal {
extern const Variant::Type s_typeTranslation[];
}

inline Variant::Type Variant::type() const
{
  return internal::s_typeTranslation[t];
}
inline Variant::SharedPtrType Variant::spType() const
{
  EPASSERT_THROW((Type)t == Type::SharedPtr, Result::InvalidType, "Variant is not a SharedPtr!");
  return (SharedPtrType)length;
}

inline bool Variant::is(Type type) const
{
  return internal::s_typeTranslation[t] == type;
}
inline bool Variant::is(SharedPtrType type) const
{
  return (Type)t == Type::SharedPtr && (SharedPtrType)length == type;
}

inline bool Variant::isValid() const
{
  return (Type)t > Type::Error;
}

inline void Variant::throwError()
{
  if ((Type)t == Type::Error)
  {
    // transfer ownership of the error object to the exception
    ErrorState *pError = err;
    err = nullptr;
    t = (size_t)Type::Void;
    throw EPException(pError);
  }
}


// *************************************************
// **        HERE BE RADIOACTIVE DRAGONS!!        **
// *************************************************
// ** template construction machinery for Variant **
// *************************************************

namespace internal {

// horrible hack to facilitate partial specialisations (support all of the types!)
template<typename T>
struct Variant_Construct
{
  static Variant construct(T &&rval);
  static Variant construct(const T &v);
};

// specialisation of non-const Variant, which annoyingly gets hooked by the T& constructor instead of the copy constructor
template<>           struct Variant_Construct<Variant>                   { epforceinline static Variant construct(const Variant &v) { return v; } };
template<size_t Len> struct Variant_Construct<MutableString<Len>>        { epforceinline static Variant construct(const MutableString<Len> &v) { return Variant(v); } };
template<>           struct Variant_Construct<SharedString>              { epforceinline static Variant construct(const SharedString &v) { return Variant(v); } };
template<size_t Len> struct Variant_Construct<Array<Variant, Len>>       { epforceinline static Variant construct(const Array<Variant, Len> &v) { return Variant(v); } };
template<>           struct Variant_Construct<SharedArray<Variant>>      { epforceinline static Variant construct(const SharedArray<Variant> &v) { return Variant(v); } };
template<>           struct Variant_Construct<Variant::VarMap>           { epforceinline static Variant construct(const Variant::VarMap &v) { return Variant(v); } };
template<size_t Len> struct Variant_Construct<Array<KeyValuePair, Len>>  { epforceinline static Variant construct(const Array<KeyValuePair, Len> &v) { return Variant(Slice<KeyValuePair>(v)); } };
template<>           struct Variant_Construct<SharedArray<KeyValuePair>> { epforceinline static Variant construct(const SharedArray<KeyValuePair> &v) { return Variant(Slice<KeyValuePair>(v)); } };

// ** suite of specialisations required to wrangle every conceivable combination of 'const'
template<typename T>
struct Variant_Construct<const T>               { epforceinline static Variant construct(const T &v) { return Variant((T&)v); } };
template<typename T, size_t N>
struct Variant_Construct<const T[N]>            { epforceinline static Variant construct(const T (&v)[N]) { return Variant_Construct<T[N]>::construct(v); } };
template<typename T>
struct Variant_Construct<const T *>             { epforceinline static Variant construct(const T *v) { return Variant((T*)v); } };

// specialisations for all the basic types
template<> struct Variant_Construct <float>     { epforceinline static Variant construct(float f)         { return Variant((double)f); } };
template<> struct Variant_Construct <int8_t>    { epforceinline static Variant construct(int8_t i)        { return Variant((int64_t)i); } };
template<> struct Variant_Construct <uint8_t>   { epforceinline static Variant construct(uint8_t i)       { return Variant((int64_t)(uint64_t)i); } };
template<> struct Variant_Construct <int16_t>   { epforceinline static Variant construct(int16_t i)       { return Variant((int64_t)i); } };
template<> struct Variant_Construct <uint16_t>  { epforceinline static Variant construct(uint16_t i)      { return Variant((int64_t)(uint64_t)i); } };
template<> struct Variant_Construct <int32_t>   { epforceinline static Variant construct(int32_t i)       { return Variant((int64_t)i); } };
template<> struct Variant_Construct <uint32_t>  { epforceinline static Variant construct(uint32_t i)      { return Variant((int64_t)(uint64_t)i); } };
template<> struct Variant_Construct <uint64_t>  { epforceinline static Variant construct(uint64_t i)      { return Variant((int64_t)i); } };
template<> struct Variant_Construct <char*>     { epforceinline static Variant construct(const char *s)   { return Variant(String(s)); } };
template<size_t N>
struct Variant_Construct <char[N]>              { epforceinline static Variant construct(const char (&s)[N]) { return Variant(String(s, N-1)); } };
template<size_t N>
struct Variant_Construct <const char[N]>        { epforceinline static Variant construct(const char (&s)[N]) { return Variant(String(s, N-1), true); } };
template<typename T, size_t N>
struct Variant_Construct <T[N]>                 { epforceinline static Variant construct(const T (&a)[N])    { return Variant(Slice<const T>(a, N)); } };


// ********************************
// ** template casting machinery **
// ********************************

template<class T>
struct is_Slice : std::false_type {};
template<class U>
struct is_Slice<Slice<U>> : std::true_type {};

// HAX: this is a horrible hax to satisfy the C++ compiler!
template<typename T>
struct Variant_Cast
{
  static_assert(!is_Slice<T>::value, "Calling Variant::as<Slice<T>> is not supported, consider using as<Array<T>> instead?");

  inline static T as(const Variant &v) { T r; epFromVariant(v, &r); return r; }
};

// partial specialisation for const
template<typename T>
struct Variant_Cast < const T >
{
  inline static const T as(const Variant &v)
  {
    return Variant_Cast<T>::as(v);
  }
};

// specialisations for Variant::as()
template<> struct Variant_Cast < bool     > { inline static bool     as(const Variant &v) { return v.asBool(); } };
template<> struct Variant_Cast < float    > { inline static float    as(const Variant &v) { return (float)v.asFloat(); } };
template<> struct Variant_Cast < double   > { inline static double   as(const Variant &v) { return v.asFloat(); } };
template<> struct Variant_Cast < char     > { inline static char     as(const Variant &v) { return (char)v.asInt(); } };
template<> struct Variant_Cast < int8_t   > { inline static int8_t   as(const Variant &v) { return (int8_t)v.asInt(); } };
template<> struct Variant_Cast < uint8_t  > { inline static uint8_t  as(const Variant &v) { return (uint8_t)v.asInt(); } };
template<> struct Variant_Cast < int16_t  > { inline static int16_t  as(const Variant &v) { return (int16_t)v.asInt(); } };
template<> struct Variant_Cast < uint16_t > { inline static uint16_t as(const Variant &v) { return (uint16_t)v.asInt(); } };
template<> struct Variant_Cast < int32_t  > { inline static int32_t  as(const Variant &v) { return (int32_t)v.asInt(); } };
template<> struct Variant_Cast < uint32_t > { inline static uint32_t as(const Variant &v) { return (uint32_t)v.asInt(); } };
template<> struct Variant_Cast < int64_t  > { inline static int64_t  as(const Variant &v) { return (int64_t)v.asInt(); } };
template<> struct Variant_Cast < uint64_t > { inline static uint64_t as(const Variant &v) { return (uint64_t)v.asInt(); } };

template<> struct Variant_Cast < Variant >              { inline static Variant              as(const Variant &v) { return v; } };
template<> struct Variant_Cast < VarDelegate >          { inline static VarDelegate          as(const Variant &v) { return v.asDelegate(); } };
template<> struct Variant_Cast < Variant::VarArray >    { inline static Variant::VarArray    as(const Variant &v) { return v.asSharedArray(); } };
template<> struct Variant_Cast < Variant::VarMap >      { inline static Variant::VarMap      as(const Variant &v) { return v.asAssocArray(); } };
template<> struct Variant_Cast < VarRange >             { inline static VarRange             as(const Variant &v) { return v.asRange(); } };

template<typename T>
struct Variant_Cast < SharedArray<T> >      { inline static SharedArray<T>     as(const Variant &v) { return v.as<Array<T, 0>>(); } };

template<>
struct Variant_Cast < String >              { inline static String             as(const Variant &v) { return v.asString(); } };
template<>
struct Variant_Cast < SharedString >        { inline static SharedString       as(const Variant &v) { return v.asSharedString(); } };
template<size_t Len>
struct Variant_Cast < MutableString<Len> >  { inline static MutableString<Len> as(const Variant &v) {
                                                                                                      if(v.is(Variant::Type::String))
                                                                                                        return MutableString<Len>(v.asString());
                                                                                                      else
                                                                                                        return MutableString<Len>(v.asSharedString()); // TODO: this double-allocation is a shame! >_<
                                                                                                    } };
template<>
struct Variant_Cast < Slice<Variant> >       { inline static Slice<Variant>       as(const Variant &v) { return v.asArray(); } };
template<>
struct Variant_Cast < Slice<const Variant> > { inline static Slice<const Variant> as(const Variant &v) { return v.asArray(); } };

// **************************************************************
// ** Variant delegates; implement typeless calling convention **
// **************************************************************

// Variant functions (this is quite complex)

template<typename Signature>
class VarDelegateMemento;

template<typename R, typename... Args>
class VarDelegateMemento<R(Args...)> : public DelegateMemento
{
protected:
  template<typename T>
  friend struct ep::SharedPtr;

  Variant to(Slice<const Variant> args) const
  {
    fastdelegate::FastDelegate<R(Args...)> d;
    d.SetMemento(target->GetFastDelegate());
    return ep::VarCall(d, args);
  }

  R from(Args... args) const
  {
    VarDelegate d(target);

    // HAX: added 1 to support the case of zero args
    Variant vargs[sizeof...(args)+1] = { Variant(args)... };
    Slice<Variant> sargs(vargs, sizeof...(args));

    Variant r = d(sargs);
    r.throwError();
    return r.as<R>();
  }

  // *to* Variant::Delegate constructor
  VarDelegateMemento(const Delegate<R(Args...)> &d)
    : target(d.GetMemento())
  {
    VarDelegate::FastDelegateType shim(this, &VarDelegateMemento::to);
    m = shim.GetMemento();
  }

  // *from* Variant::Delegate constructor
  VarDelegateMemento(const VarDelegate &d)
    : target(d.GetMemento())
  {
    FastDelegate<R(Args...)> shim(this, &VarDelegateMemento::from);
    m = shim.GetMemento();
  }

  const DelegateMementoRef target;

private:
  VarDelegateMemento<R(Args...)>& operator=(const VarDelegateMemento<R(Args...)> &rh) = delete;
};

template <typename SrcRange, typename To>
class VarRangeAdapter
{
  using From = decltype(((SrcRange*)nullptr)->getFront());

  SrcRange spRange;

public:
  VarRangeAdapter(SrcRange r) : spRange(r) {}

  RangeFeatures features() const { return spRange->features(); }

  bool empty() const { return spRange->empty(); }
  size_t length() const { return spRange->length(); }

  // TODO: this is potentially inefficient! check that these redundant constructions are optimised away
  To getFront() const { return Variant(spRange->getFront()).as<To>(); }
  void setFront(const To &value) const { spRange->setFront(Variant(value).as<From>()); }
  To popFront() { return Variant(spRange->popFront()).as<To>(); }
  void popFront(size_t n) { spRange->popFront(n); }

  To getBack() const { return Variant(spRange->getBack()).as<To>(); }
  void setBack(const To &value) const { spRange->setBack(Variant(value).as<From>()); }
  To popBack() { return Variant(spRange->popBack()).as<To>(); }
  void popBack(size_t n) { spRange->popBack(n); }

  To at(size_t index) const { return Variant(spRange->at(index)).as<To>(); }
};

} // namespace internal

// ******************************************************
// ** Take a breath; you survived, now back to sanity! **
// ******************************************************
// **                NO MORE DRAGONS!!!                **
// ******************************************************

// These pipe through to `struct Variant_Construct<>` to facilitate a bunch of partial specialisation madness
template<typename T>
epforceinline Variant::Variant(T &&rval)
  : Variant(internal::Variant_Construct<typename std::remove_reference<T>::type>::construct(std::forward<T>(rval)))
{}

template<typename T>
epforceinline T Variant::as() const
{
  // HACK: pipe this through a class so we can partial-specialise
  return internal::Variant_Cast<T>::as(*this);
}
template<>
epforceinline void Variant::as<void>() const
{
}


// *****************************************************
// ** Variant construction adapters for complex types **
// *****************************************************

// enum & bitfield keys
template<typename T, typename std::enable_if<std::is_enum<T>::value>::type* = nullptr>
inline Variant epToVariant(T e)
{
  return epToVariant(epGetValAsEnum(e));
}

// enums & bitfields
template<typename T,
  typename std::enable_if<
  std::is_base_of<Enum, T>::value ||
  std::is_base_of<Bitfield, T>::value
>::type* = nullptr>
inline Variant epToVariant(T e)
{
  return Variant(e.v, e.Desc(), std::is_base_of<Bitfield, T>::value);
}

// for arrays
template<typename T>
inline Variant epToVariant(const Slice<T> arr)
{
  Array<Variant> a(Reserve, arr.length);
  for (size_t i = 0; i<arr.length; ++i)
    a.pushBack(arr[i]);
  return std::move(a);
}

// for components
epforceinline Variant epToVariant(ComponentRef &&rval)
{
  return Variant(std::move(rval));
}
epforceinline Variant epToVariant(const ComponentRef &c)
{
  return Variant(c);
}

template<typename F>
inline Variant epToVariant(const Vector2<F> &v)
{
  Array<Variant> a(Reserve, 2);
  a.pushBack(v.x);
  a.pushBack(v.y);
  return std::move(a);
}
template<typename F>
inline Variant epToVariant(const Vector3<F> &v)
{
  Array<Variant> a(Reserve, 3);
  a.pushBack(v.x);
  a.pushBack(v.y);
  a.pushBack(v.z);
  return std::move(a);
}
template<typename F>
inline Variant epToVariant(const Vector4<F> &v)
{
  Array<Variant> a(Reserve, 4);
  a.pushBack(v.x);
  a.pushBack(v.y);
  a.pushBack(v.z);
  a.pushBack(v.w);
  return std::move(a);
}
template<typename F>
inline Variant epToVariant(const Matrix4x4<F> &m)
{
  Array<Variant> a(Reserve, 16);
  for (size_t i = 0; i<16; ++i)
    a.pushBack(m.a[i]);
  return std::move(a);
}

template<typename R, typename... Args>
inline Variant epToVariant(const Delegate<R(Args...)> &d)
{
  typedef SharedPtr<internal::VarDelegateMemento<R(Args...)>> VarDelegateRef;

  return VarDelegate(VarDelegateRef::create(d));
}

template<typename K, typename V, typename Pred>
inline Variant epToVariant(const AVLTree<K, V, Pred> &tree)
{
  Variant::VarMap m;
  for (auto item : tree)
    m.insert(item.key, item.value);
  return std::move(m);
}

template<typename Tree>
inline Variant epToVariant(const SharedMap<Tree> &map)
{
  Variant::VarMap m;
  for (auto item : map)
    m.insert(item.key, item.value);
  return std::move(m);
}

// TODO: FIX ME!!! enable_if T looks like a range!
//template<typename T>
//inline Variant epToVariant(const SharedPtr<Range<T>> &range)
//{
//  return Variant(VirtualRange<Variant>::create(range));
//}

// ***************************************************
// ** Variant conversion adapters for complex types **
// ***************************************************

// enums & bitfields
template<typename T,
  typename std::enable_if<
    std::is_base_of<Enum, T>::value ||
    std::is_base_of<Bitfield, T>::value
  >::type* = nullptr>
inline void epFromVariant(const Variant &v, T *pE)
{
  if (v.is(std::is_base_of<Bitfield, T>::value ? Variant::Type::Bitfield : Variant::Type::Enum))
  {
    size_t val;
    const EnumDesc *pDesc = v.asEnum(&val);
    EPASSERT_THROW(pDesc->name.eq(T::Name()), Result::InvalidType, "Incorrect type: enum type {0} can not convert to {1}", pDesc->name, T::Name());
    *pE = T((typename T::Type)val);
  }
  else if (v.is(Variant::Type::Int))
    *pE = T((typename T::Type)v.asInt());
  else if (v.is(Variant::Type::String))
    *pE = T(v.asString());
  else
    EPTHROW_ERROR(Result::InvalidType, "Wrong type!");
}

// math types
template<typename U>
inline void epFromVariant(const Variant &v, Vector2<U> *pR)
{
  *pR = Vector2<U>::zero();
  if (v.is(Variant::Type::Array))
  {
    auto a = v.asArray();
    if (a.length >= 2)
    {
      for (size_t i = 0; i < 2; ++i)
        ((U*)pR)[i] = a[i].as<U>();
      return;
    }
    EPTHROW_ERROR(Result::InvalidArgument, "Incorrect number of elements");
  }
  else if (v.is(Variant::SharedPtrType::AssocArray))
  {
    auto aa = v.asAssocArray();
    size_t len = v.assocArraySeriesLen();
    if (len >= 2)
    {
      size_t start = aa.get(0) ? 0 : 1;
      for (size_t i = 0; i < 2; ++i)
        ((U*)pR)[i] = aa.get(start + i)->as<U>();
      return;
    }
    pR->y = aa["y"].as<U>();
    pR->x = aa["x"].as<U>();
  }
  EPTHROW_ERROR(Result::InvalidType, "Wrong type!");
}
template<typename U>
inline void epFromVariant(const Variant &v, Vector3<U> *pR)
{
  *pR = Vector3<U>::zero();
  if (v.is(Variant::Type::Array))
  {
    auto a = v.asArray();
    if (a.length >= 3)
    {
      for (size_t i = 0; i < 3; ++i)
        ((U*)pR)[i] = a[i].as<U>();
      return;
    }
    EPTHROW_ERROR(Result::InvalidArgument, "Incorrect number of elements");
  }
  else if (v.is(Variant::SharedPtrType::AssocArray))
  {
    auto aa = v.asAssocArray();
    size_t len = v.assocArraySeriesLen();
    if (len >= 3)
    {
      size_t start = aa.get(0) ? 0 : 1;
      for (size_t i = 0; i < 3; ++i)
        ((U*)pR)[i] = aa.get(start + i)->as<U>();
      return;
    }
    pR->z = aa["z"].as<U>();
    pR->y = aa["y"].as<U>();
    pR->x = aa["x"].as<U>();
  }
  EPTHROW_ERROR(Result::InvalidType, "Wrong type!");
}
template<typename U>
inline void epFromVariant(const Variant &v, Vector4<U> *pR)
{
  *pR = Vector4<U>::zero();
  if (v.is(Variant::Type::Array))
  {
    auto a = v.asArray();
    if (a.length >= 4)
    {
      for (size_t i = 0; i < 4; ++i)
        ((U*)pR)[i] = a[i].as<U>();
      return;
    }
    EPTHROW_ERROR(Result::InvalidArgument, "Incorrect number of elements");
  }
  else if (v.is(Variant::SharedPtrType::AssocArray))
  {
    auto aa = v.asAssocArray();
    size_t len = v.assocArraySeriesLen();
    if (len >= 4)
    {
      size_t start = aa.get(0) ? 0 : 1;
      for (size_t i = 0; i < 4; ++i)
        ((U*)pR)[i] = aa.get(start + i)->as<U>();
      return;
    }
    pR->w = aa["w"].as<U>();
    pR->z = aa["z"].as<U>();
    pR->y = aa["y"].as<U>();
    pR->x = aa["x"].as<U>();
  }
  EPTHROW_ERROR(Result::InvalidType, "Wrong type!");
}
template<typename U>
inline void epFromVariant(const Variant &v, Matrix4x4<U> *pR)
{
  *pR = Matrix4x4<U>::identity();
  if (v.is(Variant::Type::Array))
  {
    auto a = v.asArray();
    if (a.length == 16)
    {
      for (size_t i = 0; i < 16; ++i)
        ((U*)pR)[i] = a[i].as<U>();
      return;
    }
    EPTHROW_ERROR(Result::InvalidArgument, "Incorrect number of elements");
  }
  else if (v.is(Variant::SharedPtrType::AssocArray))
  {
    auto aa = v.asAssocArray();
    size_t len = v.assocArraySeriesLen();
    if (len == 16)
    {
      size_t start = aa.get(0) ? 0 : 1;
      for (size_t i = 0; i < 16; ++i)
        ((U*)pR)[i] = aa.get(start + i)->as<U>();
      return;
    }
    EPTHROW_ERROR(Result::InvalidArgument, "Incorrect number of elements");
  }
  EPTHROW_ERROR(Result::InvalidType, "Wrong type!");
}

template<typename R, typename... Args>
inline void epFromVariant(const Variant &v, Delegate<R(Args...)> *pD)
{
  typedef SharedPtr<internal::VarDelegateMemento<R(Args...)>> VarDelegateRef;
  *pD = Delegate<R(Args...)>(VarDelegateRef::create(v.asDelegate()));
}

// TODO: FIX ME!!! enable_if T looks like a range!
//template<typename T>
//inline void epFromVariant(const Variant &v, SharedPtr<Range<T>> *pRange)
//{
//  *pRange = SharedPtr<internal::VarRangeAdapter<Variant, T>>::create(v.asRange());
//}

template<typename U, size_t Len>
inline void epFromVariant(const Variant &v, Array<U, Len> *pArr)
{
  pArr->clear();
  if (v.is(Variant::Type::Array))
  {
    auto a = v.asArray();
    pArr->reserve(a.length);
    for (size_t i = 0; i < a.length; ++i)
      pArr->pushBack(a[i].as<U>());
    return;
  }
  else if (v.is(Variant::SharedPtrType::AssocArray))
  {
    size_t len = v.assocArraySeriesLen();
    if (len > 0)
    {
      Variant::VarMap m = v.asAssocArray();
      pArr->reserve(len);
      size_t start = m.get(0) ? 0 : 1;
      for (size_t i = 0; i < len; ++i)
        pArr->pushBack(m.get(start + i)->as<U>());
    }
    return;
  }
  else if (v.is(Variant::Type::String))
  {
    String s = v.asString();
    s = s.trim();
    if (s[0] == '[' && s[s.length-1] == ']')
    {
      s.popFront(); s.popBack();
      size_t offset;
      do
      {
        offset = s.findFirst(',');
        pArr->pushBack(Variant(s.slice(0, offset), true).as<U>());
        s.pop(offset < s.length ? offset + 1 : offset);
      } while (s);
      return;
    }
    EPTHROW_ERROR(Result::InvalidArgument, "String does not look like an array, ie: \"[x, y, z]\"");
  }
  else if (v.is(Variant::Type::Null))
    return;
  EPTHROW_ERROR(Result::InvalidType, "Wrong type!");
}

template<typename K, typename V, typename Pred>
inline void epFromVariant(const Variant &v, AVLTree<K, V, Pred> *pTree)
{
  if (v.is(Variant::Type::Array))
  {
    auto a = v.asArray();
    for (size_t i = 0; i < a.length; ++i)
      pTree->insert(Variant(i).as<K>(), a[i].as<V>());
    return;
  }
  else if (v.is(Variant::SharedPtrType::AssocArray))
  {
    auto aa = v.asAssocArray();
    for (auto kvp : aa)
      pTree->insert(kvp.key.as<K>(), kvp.value.as<V>());
    return;
  }
  else if (v.is(Variant::Type::Null))
    return;
  EPTHROW_ERROR(Result::InvalidType, "Wrong type!");
}

template<typename Tree>
inline void epFromVariant(const Variant &v, SharedMap<Tree> *pTree)
{
  if (v.is(Variant::Type::Array))
  {
    auto a = v.asArray();
    for (size_t i = 0; i < a.length; ++i)
      pTree->insert(Variant(i).as<typename Tree::KeyType>(), a[i].as<typename Tree::ValueType>());
    return;
  }
  else if (v.is(Variant::SharedPtrType::AssocArray))
  {
    auto aa = v.asAssocArray();
    for (auto kvp : aa)
      pTree->insert(kvp.key.as<typename Tree::KeyType>(), kvp.value.as<typename Tree::ValueType>());
    return;
  }
  else if (v.is(Variant::Type::Null))
    return;
  EPTHROW_ERROR(Result::InvalidType, "Wrong type!");
}

template<>
struct AVLTreeAllocator<VariantAVLNode>;

namespace internal
{
  AVLTreeAllocator<VariantAVLNode> &GetAVLTreeAllocator();
}

template<>
struct AVLTreeAllocator<VariantAVLNode>
{
  AVLTreeAllocator() : nodes(1024) {} // TODO: Revisit this to see if 1024 is appropriate.
  using Node = VariantAVLNode;

  Node *Alloc()
  {
    return nodes.Alloc();
  }

  void Free(Node *pMem)
  {
    nodes.Free(pMem);
  }

  static AVLTreeAllocator &Get() { return internal::GetAVLTreeAllocator(); }

  FreeList<Node> nodes;
};

namespace internal {

template<typename T>
epforceinline Variant Variant_Construct<T>::construct(T &&rval)
{
  return epToVariant(std::move(rval));
}

template<typename T>
epforceinline Variant Variant_Construct<T>::construct(const T &v)
{
  return epToVariant(v);
}

} // internal
} // ep
