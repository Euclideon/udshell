
#include <type_traits>

#include "udMath.h"

namespace udKernel
{

// constructors...
inline Variant::Variant()
  : t((size_t)Type::Null)
  , ownsArray(0)
  , length(0)
{}

inline Variant::Variant(Variant &&rval)
  : t(rval.t)
  , ownsArray(rval.ownsArray)
  , length(rval.length)
  , p(rval.p)
{
  rval.t = (size_t)Type::Null;
}

inline Variant::Variant(const Variant &val)
  : t(val.t)
  , ownsArray(val.ownsArray)
  , length(val.length)
  , p(val.p)
{
  if (is(Type::Delegate))
  {
    new((void*)&p) Delegate((Delegate&)val.p);
  }
  else if (ownsArray)
  {
    if (is(Type::Array))
    {
      a = (Variant*)udAlloc(sizeof(Variant)*length);
      for (size_t i = 0; i<length; ++i)
        new((void*)&a[i]) Variant(val.a[i]);
    }
    else if (is(Type::AssocArray))
    {
      aa = (KeyValuePair*)udAlloc(sizeof(KeyValuePair)*length);
      for (size_t i = 0; i<length; ++i)
      {
        new((void*)&aa[i].key) Variant(val.aa[i].key);
        new((void*)&aa[i].value) Variant(val.aa[i].value);
      }
    }
  }
}

inline Variant::Variant(bool b)
  : t((size_t)Type::Bool)
  , ownsArray(0)
  , length(0)
  , b(b)
{}
inline Variant::Variant(int64_t i)
  : t((size_t)Type::Int)
  , ownsArray(0)
  , length(0)
  , i(i)
{}
inline Variant::Variant(double f)
  : t((size_t)Type::Float)
  , ownsArray(0)
  , length(0)
  , f(f)
{}
inline Variant::Variant(Component *c)
  : t((size_t)Type::Component)
  , ownsArray(0)
  , length(0)
  , c(c)
{}
inline Variant::Variant(const Delegate &d)
  : t((size_t)Type::Delegate)
  , ownsArray(0)
  , length(0)
{
  new(&p) Delegate(d);
}
inline Variant::Variant(Delegate &&d)
  : t((size_t)Type::Delegate)
  , ownsArray(0)
  , length(0)
{
  new(&p) Delegate(std::move(d));
}
inline Variant::Variant(udString s)
  : t((size_t)Type::String)
  , ownsArray(0)
  , length(s.length)
  , s(s.ptr)
{}
inline Variant::Variant(udSlice<Variant> a, bool ownsMemory)
  : t((size_t)Type::Array)
  , ownsArray(ownsMemory ? 1 : 0)
  , length(a.length)
  , a(a.ptr)
{}
inline Variant::Variant(udSlice<KeyValuePair> aa, bool ownsMemory)
  : t((size_t)Type::AssocArray)
  , ownsArray(ownsMemory ? 1 : 0)
  , length(aa.length)
  , aa(aa.ptr)
{}

// destructor
inline Variant::~Variant()
{
  if (is(Type::Delegate))
  {
    ((Delegate*)&p)->~Delegate();
  }
  else if (ownsArray && t >= (size_t)Type::Array)
  {
    if (is(Type::Array))
    {
      for (size_t i = 0; i < length; ++i)
        a[i].~Variant();
    }
    else if (is(Type::AssocArray))
    {
      for (size_t i = 0; i < length; ++i)
      {
        aa[i].key.~Variant();
        aa[i].value.~Variant();
      }
    }
    udFree(a);
  }
}

inline Variant& Variant::operator=(Variant &&rval)
{
  if (this != &rval)
  {
    this->~Variant();

    t = rval.t;
    ownsArray = rval.ownsArray;
    length = rval.length;
    p = rval.p;

    rval.ownsArray = false;
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


// *************************************
// ** template construction machinery **
// *************************************

// horrible hack to facilitate partial specialisations (support all of the types!)
template<typename T>
struct Variant_Construct
{
  inline static Variant construct(const T &v)
  {
    return Variant(ToVariant(v));
  }
};
template<typename T>
Variant::Variant(const T &v)
  : Variant(Variant_Construct<typename std::remove_const<T>::type>::construct(v))
{}
template<typename T>
Variant::Variant(T &v)
  : Variant((const T&)v)
{}

// specialisations of Variant_Construct for all the basic types
template<> struct Variant_Construct <nullptr_t> { inline static Variant construct(nullptr_t)       { return Variant(); } };
template<> struct Variant_Construct <float>     { inline static Variant construct(float f)         { return Variant((double)f); } };
template<> struct Variant_Construct <int8_t>    { inline static Variant construct(int8_t i)        { return Variant((int64_t)i); } };
template<> struct Variant_Construct <uint8_t>   { inline static Variant construct(uint8_t i)       { return Variant((int64_t)(uint64_t)i); } };
template<> struct Variant_Construct <int16_t>   { inline static Variant construct(int16_t i)       { return Variant((int64_t)i); } };
template<> struct Variant_Construct <uint16_t>  { inline static Variant construct(uint16_t i)      { return Variant((int64_t)(uint64_t)i); } };
template<> struct Variant_Construct <int32_t>   { inline static Variant construct(int32_t i)       { return Variant((int64_t)i); } };
template<> struct Variant_Construct <uint32_t>  { inline static Variant construct(uint32_t i)      { return Variant((int64_t)(uint64_t)i); } };
template<> struct Variant_Construct <uint64_t>  { inline static Variant construct(uint64_t i)      { return Variant((int64_t)i); } };
template<> struct Variant_Construct <char*>     { inline static Variant construct(const char *s)   { return Variant(udString(s)); } };
template<size_t N>
struct Variant_Construct <char[N]>              { inline static Variant construct(const char s[N]) { return Variant(udString(s, N-1)); } };

// for components
inline Variant ToVariant(const ComponentRef c)
{
  return Variant(c.ptr());
}

// vectors and matrices (require partial specialisation)
template<typename F>
inline Variant ToVariant(const udVector2<F> &v)
{
  Variant r;
  Variant *a = r.allocArray(2);
  new(&a[0]) Variant(v.x);
  new(&a[1]) Variant(v.y);
  return r;
}
template<typename F>
inline Variant ToVariant(const udVector3<F> &v)
{
  Variant r;
  Variant *a = r.allocArray(3);
  new(&a[0]) Variant(v.x);
  new(&a[1]) Variant(v.y);
  new(&a[2]) Variant(v.z);
  return r;
}
template<typename F>
inline Variant ToVariant(const udVector4<F> &v)
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
inline Variant ToVariant(const udMatrix4x4<F> &m)
{
  Variant r;
  Variant *a = r.allocArray(16);
  for (size_t i = 0; i<16; ++i)
    new(&a[i]) Variant(m.a[i]);
  return r;
}


// functions (this is quite complex)

// these horrid templates generate an integer sequence
template<size_t ...> struct Sequence { };
template<int N, size_t ...S> struct GenSequence : GenSequence<N-1, N-1, S...> { };
template<size_t ...S> struct GenSequence<0, S...> { typedef Sequence<S...> type; };

template<typename Signature>
class VarDelegate;

template<typename R, typename... Args>
class VarDelegate<R(Args...)> : public DelegateMemento
{
protected:
  template<typename T>
  friend class SharedPtr;

  template<size_t ...S>
  UDFORCE_INLINE static Variant callFuncHack(udSlice<Variant> args, const udDelegate<R(Args...)> &d, Sequence<S...>)
  {
    return Variant(d(args[S].as<Args>()...));
  }

  Variant to(udSlice<Variant> args) const
  {
    // we need a call shim which can give an integer sequence as S... (Bjarne!!!)
    return callFuncHack(args, udDelegate<R(Args...)>(target), typename GenSequence<sizeof...(Args)>::type());
  }

  R from(Args... args) const
  {
    Variant::Delegate d(target);

    // HAX: added 1 to support the case of zero args
    Variant vargs[sizeof...(args)+1] = { Variant(args)... };
    udSlice<Variant> sargs(vargs, sizeof...(args));

    return d(sargs).as<R>();
  }

  // *to* Variant::Delegate constructor
  VarDelegate(const udDelegate<R(Args...)> &d)
    : target(d.GetMemento())
  {
    FastDelegate<Variant(udSlice<Variant>)> shim(this, &VarDelegate::Partial<R, Args...>::to);
    m = shim.GetMemento();
  }

  // *from* Variant::Delegate constructor
  VarDelegate(const Variant::Delegate &d)
    : target(d.GetMemento())
  {
    FastDelegate<R(Args...)> shim(this, &VarDelegate::Partial<R, Args...>::from);
    m = shim.GetMemento();
  }

  const DelegateMementoRef target;
};

// specialise for 'void' return type
template<typename... Args>
class VarDelegate<void(Args...)> : public DelegateMemento
{
protected:
  template<typename T>
  friend class SharedPtr;

  template<size_t ...S>
  UDFORCE_INLINE static void callFuncHack(udSlice<Variant> args, const udDelegate<void(Args...)> &d, Sequence<S...>)
  {
    d(args[S].as<Args>()...);
  }

  Variant to(udSlice<Variant> args) const
  {
    // we need a call shim which can give an integer sequence as S... (Bjarne!!!)
    callFuncHack(args, udDelegate<void(Args...)>(target), typename GenSequence<sizeof...(Args)>::type());
    return Variant();
  }

  void from(Args... args) const
  {
    Variant::Delegate d(target);

    // HAX: added 1 to support the case of zero args
    Variant vargs[sizeof...(args)+1] = { Variant(args)... };
    udSlice<Variant> sargs(vargs, sizeof...(args));

    d(sargs);
  }

  // *to* Variant::Delegate constructor
  VarDelegate(const udDelegate<void(Args...)> &d)
    : target(d.GetMemento())
  {
    FastDelegate<Variant(udSlice<Variant>)> shim(this, &VarDelegate::to);
    m = shim.GetMemento();
  }

  // *from* Variant::Delegate constructor
  VarDelegate(const Variant::Delegate &d)
    : target(d.GetMemento())
  {
    FastDelegate<void(Args...)> shim(this, &VarDelegate::from);
    m = shim.GetMemento();
  }

  const DelegateMementoRef target;
};

template<typename R, typename... Args>
inline Variant ToVariant(const udDelegate<R(Args...)> &d)
{
  typedef SharedPtr<VarDelegate<R(Args...)> VarDelegateRef;

  return Variant::Delegate(VarDelegateRef::create(d));
}


// ********************************
// ** template casting machinery **
// ********************************

// HAX: this is a horrible hax to satisfy the C++ compiler!
template<typename T>
struct Variant_Cast
{
  inline static T as(const Variant &v) { T r; udFromVariant(v, &r); return r; }
};

template<typename T>
inline T Variant::as() const
{
  // HACK: pipe this through a class so we can partial-specialise
  return Variant_Cast<T>::as(*this);
}

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
template<> struct Variant_Cast < udString > { inline static udString as(const Variant &v) { return v.asString(); } };

// udMath types
template<typename U>
inline void udFromVariant(const Variant &v, udVector2<U> *pR)
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
inline void udFromVariant(const Variant &v, udVector3<U> *pR)
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
inline void udFromVariant(const Variant &v, udVector4<U> *pR)
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
inline void udFromVariant(const Variant &v, udMatrix4x4<U> *pR)
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
inline void udFromVariant(const Variant &v, udDelegate<R(Args...)> *pD)
{
  typedef SharedPtr<VarDelegate<R(Args...)>> VarDelegateRef;

  *pD = udDelegate<R(Args...)>(VarDelegateRef::create(v.asDelegate()));
}
} // namespace udKernel
