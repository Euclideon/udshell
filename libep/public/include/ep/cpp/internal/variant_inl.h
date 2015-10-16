#include <type_traits>

#include "udMath.h"

ptrdiff_t epStringifyVariant(Slice<char> buffer, String format, const Variant &var, const epVarArg*);

namespace ep {
namespace internal {

template<> struct StringifyProxy<Variant>
{
  inline static ptrdiff_t stringify(Slice<char> buffer, String format, const void *pData, const epVarArg *pArgs) { return epStringifyVariant(buffer, format, *(Variant*)pData, pArgs); }
  inline static int64_t intify(const void *pData) { return ((Variant*)pData)->asInt(); }
};

} // namespace internal

// constructors...
inline Variant::Variant()
{
  t = (size_t)Type::Null;
  ownsContent = 0;
  length = 0;
}

inline Variant::Variant(Variant &&rval)
{
  t = rval.t;
  ownsContent = rval.ownsContent;
  length = rval.length;
  p = rval.p;

  rval.t = (size_t)Type::Null;
  rval.ownsContent = 0;
}

inline Variant::Variant(const Variant &val)
{
  t = val.t;
  ownsContent = val.ownsContent;
  length = val.length;
  p = val.p;

  if (ownsContent)
  {
    if (is(Type::Component))
    {
      new((void*)&p) ComponentRef((ComponentRef&)val.p);
    }
    else if (is(Type::Delegate))
    {
      new((void*)&p) VarDelegate((VarDelegate&)val.p);
    }
    else if (is(Type::String))
    {
      char *pS = (char*)epAlloc(length);
      memcpy(pS, val.s, length);
      s = pS;
    }
    else if (is(Type::Array))
    {
      Variant *a = (Variant*)epAlloc(sizeof(Variant)*length);
      for (size_t i = 0; i<length; ++i)
        new((void*)&a[i]) Variant(((const Variant*)val.p)[i]);
      p = a;
    }
    else if (is(Type::AssocArray))
    {
      KeyValuePair *aa = (KeyValuePair*)epAlloc(sizeof(KeyValuePair)*length);
      for (size_t i = 0; i<length; ++i)
      {
        new((void*)&aa[i].key) Variant((const Variant&)((KeyValuePair*)val.p)[i].key);
        new((void*)&aa[i].value) Variant((const Variant&)((KeyValuePair*)val.p)[i].value);
      }
      p = aa;
    }
  }
}

inline Variant::Variant(epVariant &&rval)
  : Variant(std::move((Variant&)rval))
{}
inline Variant::Variant(const epVariant &val)
  : Variant((Variant&)val)
{}

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
inline Variant::Variant(size_t val, const epEnumDesc *pDesc, bool isBitfield)
{
  t = isBitfield ? (size_t)Type::Bitfield : (size_t)Type::Enum;
  ownsContent = 0;
  length = val;
  p = (void*)pDesc;
}
inline Variant::Variant(ComponentRef &&spC)
{
  t = (size_t)Type::Component;
  ownsContent = 1;
  length = 0;
  new(&p) ComponentRef(std::move(spC));
}
inline Variant::Variant(const ComponentRef &spC)
{
  t = (size_t)Type::Component;
  ownsContent = 1;
  length = 0;
  new(&p) ComponentRef(spC);
}
inline Variant::Variant(const VarDelegate &d)
{
  t = (size_t)Type::Delegate;
  ownsContent = 1;
  length = 0;
  new(&p) VarDelegate(d);
}
inline Variant::Variant(VarDelegate &&d)
{
  t = (size_t)Type::Delegate;
  ownsContent = 1;
  length = 0;
  new(&p) VarDelegate(std::move(d));
}
inline Variant::Variant(String s, bool ownsMemory)
{
  t = (size_t)Type::String;
  ownsContent = ownsMemory ? 1 : 0;
  length = s.length;
  this->s = s.ptr;
}
inline Variant::Variant(Slice<Variant> a, bool ownsMemory)
{
  t = (size_t)Type::Array;
  ownsContent = ownsMemory ? 1 : 0;
  length = a.length;
  p = a.ptr;
}
inline Variant::Variant(Slice<KeyValuePair> aa, bool ownsMemory)
{
  t = (size_t)Type::AssocArray;
  ownsContent = ownsMemory ? 1 : 0;
  length = aa.length;
  p = aa.ptr;
}

inline Variant& Variant::operator=(Variant &&rval)
{
  if (this != &rval)
  {
    this->~Variant();

    t = rval.t;
    ownsContent = rval.ownsContent;
    length = rval.length;
    p = rval.p;

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
    new(this) Variant(rval);
  }
  return *this;
}

inline Variant::Type Variant::type() const
{
  return (Type)t;
}

inline bool Variant::is(Type type) const
{
  return (Type)t == type;
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
  epforceinline static Variant construct(T &&rval)
  {
    return ::epToVariant(std::move(rval));
  }
  epforceinline static Variant construct(const T &v)
  {
    return ::epToVariant(v);
  }
};

// specialisation of non-const Variant, which annoyingly gets hooked by the T& constructor instead of the copy constructor
template<> struct Variant_Construct<Variant>    { epforceinline static Variant construct(const Variant &v) { return Variant(v); } };

// ** suite of specialisations required to wrangle every conceivable combination of 'const'
template<typename T>
struct Variant_Construct<const T>               { epforceinline static Variant construct(const T &v) { return Variant((T&)v); } };
template<typename T, size_t N>
struct Variant_Construct<const T[N]>            { epforceinline static Variant construct(const T v[N]) { return Variant_Construct<T[N]>::construct(v); } };
template<typename T>
struct Variant_Construct<const T *>             { epforceinline static Variant construct(const T *v) { return Variant((T*)v); } };

// specialisations for all the basic types
template<> struct Variant_Construct <nullptr_t> { epforceinline static Variant construct(nullptr_t)       { return Variant(); } };
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
struct Variant_Construct <char[N]>              { epforceinline static Variant construct(const char s[N]) { return Variant(String(s, N-1)); } };
template<typename T, size_t N>
struct Variant_Construct <T[N]>                 { epforceinline static Variant construct(const T a[N])    { return Variant(Slice<T>(a, N)); } };


// ********************************
// ** template casting machinery **
// ********************************

// HAX: this is a horrible hax to satisfy the C++ compiler!
template<typename T>
struct Variant_Cast
{
  inline static T as(const Variant &v) { T r; ::epFromVariant(v, &r); return r; }
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

// specialisations for udVeriant::as()
template<> struct Variant_Cast < bool     > { inline static bool     as(const Variant &v) { return v.asBool(); } };
template<> struct Variant_Cast < float    > { inline static float    as(const Variant &v) { return (float)v.asFloat(); } };
template<> struct Variant_Cast < double   > { inline static double   as(const Variant &v) { return v.asFloat(); } };
template<> struct Variant_Cast < int8_t   > { inline static int8_t   as(const Variant &v) { return (int8_t)v.asInt(); } };
template<> struct Variant_Cast < uint8_t  > { inline static uint8_t  as(const Variant &v) { return (uint8_t)v.asInt(); } };
template<> struct Variant_Cast < int16_t  > { inline static int16_t  as(const Variant &v) { return (int16_t)v.asInt(); } };
template<> struct Variant_Cast < uint16_t > { inline static uint16_t as(const Variant &v) { return (uint16_t)v.asInt(); } };
template<> struct Variant_Cast < int32_t  > { inline static int32_t  as(const Variant &v) { return (int32_t)v.asInt(); } };
template<> struct Variant_Cast < uint32_t > { inline static uint32_t as(const Variant &v) { return (uint32_t)v.asInt(); } };
template<> struct Variant_Cast < int64_t  > { inline static int64_t  as(const Variant &v) { return (int64_t)v.asInt(); } };
template<> struct Variant_Cast < uint64_t > { inline static uint64_t as(const Variant &v) { return (uint64_t)v.asInt(); } };
template<> struct Variant_Cast < String >   { inline static String   as(const Variant &v) { return v.asString(); } };

template<> struct Variant_Cast < Variant >  { inline static Variant  as(const Variant &v) { return v; } };


// **************************************************************
// ** Variant delegates; implement typeless calling convention **
// **************************************************************

// Variant functions (this is quite complex)

// these horrid templates generate an integer sequence
template<size_t ...> struct Sequence { };
template<int N, size_t ...S> struct GenSequence : GenSequence<N-1, N-1, S...> { };
template<size_t ...S> struct GenSequence<0, S...> { typedef Sequence<S...> type; };

template<typename Signature>
class VarDelegateMemento;

template<typename R, typename... Args>
class VarDelegateMemento<R(Args...)> : public DelegateMemento
{
protected:
  template<typename T>
  friend class SharedPtr;

  template<size_t ...S>
  epforceinline static Variant callFuncHack(Slice<Variant> args, const Delegate<R(Args...)> &d, Sequence<S...>)
  {
    return Variant(d(args[S].as<typename std::remove_reference<Args>::type>()...));
  }

  Variant to(Slice<Variant> args) const
  {
    // we need a call shim which can give an integer sequence as S... (Bjarne!!!)
    return callFuncHack(args, Delegate<R(Args...)>(target), typename GenSequence<sizeof...(Args)>::type());
  }

  R from(Args... args) const
  {
    Variant::VarDelegate d(target);

    // HAX: added 1 to support the case of zero args
    Variant vargs[sizeof...(args)+1] = { Variant(args)... };
    Slice<Variant> sargs(vargs, sizeof...(args));

    return d(sargs).as<R>();
  }

  // *to* Variant::Delegate constructor
  VarDelegateMemento(const Delegate<R(Args...)> &d)
    : target(d.GetMemento())
  {
    FastDelegate<Variant(Slice<Variant>)> shim(this, &VarDelegateMemento::to);
    m = shim.GetMemento();
  }

  // *from* Variant::Delegate constructor
  VarDelegateMemento(const Variant::VarDelegate &d)
    : target(d.GetMemento())
  {
    FastDelegate<R(Args...)> shim(this, &VarDelegateMemento::from);
    m = shim.GetMemento();
  }

  const DelegateMementoRef target;

private:
  VarDelegateMemento<R(Args...)>& operator=(const VarDelegateMemento<R(Args...)> &rh) = delete;
};

// specialise for 'void' return type
template<typename... Args>
class VarDelegateMemento<void(Args...)> : public DelegateMemento
{
protected:
  template<typename T>
  friend class SharedPtr;

  template<size_t ...S>
  epforceinline static void callFuncHack(Slice<Variant> args, const Delegate<void(Args...)> &d, Sequence<S...>)
  {
    d(args[S].as<typename std::remove_reference<Args>::type>()...);
  }

  Variant to(Slice<Variant> args) const
  {
    // we need a call shim which can give an integer sequence as S... (Bjarne!!!)
    callFuncHack(args, Delegate<void(Args...)>(target), typename GenSequence<sizeof...(Args)>::type());
    return Variant();
  }

  void from(Args... args) const
  {
    Variant::VarDelegate d(target);

    // HAX: added 1 to support the case of zero args
    Variant vargs[sizeof...(args)+1] = { Variant(args)... };
    Slice<Variant> sargs(vargs, sizeof...(args));

    d(sargs);
  }

  // *to* Variant::Delegate constructor
  VarDelegateMemento(const Delegate<void(Args...)> &d)
    : target(d.GetMemento())
  {
    FastDelegate<Variant(Slice<Variant>)> shim(this, &VarDelegateMemento::to);
    m = shim.GetMemento();
  }

  // *from* Variant::Delegate constructor
  VarDelegateMemento(const Variant::VarDelegate &d)
    : target(d.GetMemento())
  {
    FastDelegate<void(Args...)> shim(this, &VarDelegateMemento::from);
    m = shim.GetMemento();
  }

  const DelegateMementoRef target;

private:
  VarDelegateMemento<void(Args...)>& operator=(const VarDelegateMemento<void(Args...)> &rh) = delete;
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

} // namespace ep



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
  std::is_base_of<epEnum, T>::value ||
  std::is_base_of<epBitfield, T>::value
>::type* = nullptr>
inline Variant epToVariant(T e)
{
  return Variant(e.v, e.Desc(), std::is_base_of<epBitfield, T>::value);
}

// for arrays
template<typename T>
inline Variant epToVariant(const Slice<T> arr)
{
  Variant r;
  Variant *a = r.allocArray(arr.length);
  for (size_t i = 0; i<arr.length; ++i)
    new(&a[i]) Variant(arr[i]);
  return r;
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
inline Variant epToVariant(const udVector2<F> &v)
{
  Variant r;
  Variant *a = r.allocArray(2);
  new(&a[0]) Variant(v.x);
  new(&a[1]) Variant(v.y);
  return r;
}
template<typename F>
inline Variant epToVariant(const udVector3<F> &v)
{
  Variant r;
  Variant *a = r.allocArray(3);
  new(&a[0]) Variant(v.x);
  new(&a[1]) Variant(v.y);
  new(&a[2]) Variant(v.z);
  return r;
}
template<typename F>
inline Variant epToVariant(const udVector4<F> &v)
{
  Variant r;
  Variant *a = r.allocArray(4);
  new(&a[0]) Variant(v.x);
  new(&a[1]) Variant(v.y);
  new(&a[2]) Variant(v.z);
  new(&a[3]) Variant(v.w);
  return r;
}
template<typename F>
inline Variant epToVariant(const udMatrix4x4<F> &m)
{
  Variant r;
  Variant *a = r.allocArray(16);
  for (size_t i = 0; i<16; ++i)
    new(&a[i]) Variant(m.a[i]);
  return r;
}

template<typename R, typename... Args>
inline Variant epToVariant(const Delegate<R(Args...)> &d)
{
  typedef SharedPtr<internal::VarDelegateMemento<R(Args...)>> VarDelegateRef;

  return Variant::VarDelegate(VarDelegateRef::create(d));
}


// ***************************************************
// ** Variant conversion adapters for complex types **
// ***************************************************

// enums & bitfields
template<typename T,
  typename std::enable_if<
    std::is_base_of<epEnum, T>::value ||
    std::is_base_of<epBitfield, T>::value
  >::type* = nullptr>
inline void epFromVariant(const Variant &v, T *pE)
{
  if (v.is(std::is_base_of<epBitfield, T>::value ? Variant::Type::Bitfield : Variant::Type::Enum))
  {
    size_t val;
    const epEnumDesc *pDesc = v.asEnum(&val);
    if (!pDesc->name.eq(T::Name()))
    {
      // TODO: complain about invalid enum type?! error or something?
      return;
    }
    *pE = T((typename T::Type)val);
  }
  else if (v.is(Variant::Type::Int))
    *pE = T((typename T::Type)v.asInt());
  else if (v.is(Variant::Type::String))
    *pE = T(v.asString());
}

// udMath types
template<typename U>
inline void epFromVariant(const Variant &v, udVector2<U> *pR)
{
  *pR = udVector2<U>::zero();
  if (v.is(Variant::Type::Array))
  {
    auto a = v.asArray();
    if (a.length >= 2)
    {
      for (size_t i = 0; i < 2; ++i)
        ((U*)pR)[i] = (U)a[i].asFloat();
    }
  }
  else if (v.is(Variant::Type::AssocArray))
  {
    auto aa = v.asAssocArraySeries();
    if (aa.length >= 2)
    {
      for (size_t i = 0; i < 2; ++i)
        ((U*)pR)[i] = (U)aa[i].value.asFloat();
    }
  }
}
template<typename U>
inline void epFromVariant(const Variant &v, udVector3<U> *pR)
{
  *pR = udVector3<U>::zero();
  if (v.is(Variant::Type::Array))
  {
    auto a = v.asArray();
    if (a.length >= 3)
    {
      for (size_t i = 0; i < 3; ++i)
        ((U*)pR)[i] = (U)a[i].asFloat();
    }
  }
  else if (v.is(Variant::Type::AssocArray))
  {
    auto aa = v.asAssocArraySeries();
    if (aa.length >= 3)
    {
      for (size_t i = 0; i < 3; ++i)
        ((U*)pR)[i] = (U)aa[i].value.asFloat();
    }
  }
}
template<typename U>
inline void epFromVariant(const Variant &v, udVector4<U> *pR)
{
  *pR = udVector4<U>::zero();
  if (v.is(Variant::Type::Array))
  {
    auto a = v.asArray();
    if (a.length >= 4)
    {
      for (size_t i = 0; i < 4; ++i)
        ((U*)pR)[i] = (U)a[i].asFloat();
    }
  }
  else if (v.is(Variant::Type::AssocArray))
  {
    auto aa = v.asAssocArraySeries();
    if (aa.length >= 4)
    {
      for (size_t i = 0; i < 4; ++i)
        ((U*)pR)[i] = (U)aa[i].value.asFloat();
    }
  }
}
template<typename U>
inline void epFromVariant(const Variant &v, udMatrix4x4<U> *pR)
{
  *pR = udMatrix4x4<U>::identity();
  if (v.is(Variant::Type::Array))
  {
    auto a = v.asArray();
    if (a.length >= 16)
    {
      for (size_t i = 0; i < 16; ++i)
        ((U*)pR)[i] = (U)a[i].asFloat();
    }
  }
  else if (v.is(Variant::Type::AssocArray))
  {
    auto aa = v.asAssocArraySeries();
    if (aa.length >= 16)
    {
      for (size_t i = 0; i < 16; ++i)
        ((U*)pR)[i] = (U)aa[i].value.asFloat();
    }
  }
}

template<typename R, typename... Args>
inline void epFromVariant(const Variant &v, Delegate<R(Args...)> *pD)
{
  typedef SharedPtr<internal::VarDelegateMemento<R(Args...)>> VarDelegateRef;

  if (v.asDelegate())
    *pD = Delegate<R(Args...)>(VarDelegateRef::create(v.asDelegate()));
  else
    *pD = nullptr;
}
