
#include <type_traits>

// constructors...
inline udVariant::udVariant()
  : t((size_t)Type::Null)
  , ownsContent(0)
  , length(0)
{}

inline udVariant::udVariant(udVariant &&rval)
  : t(rval.t)
  , ownsContent(rval.ownsContent)
  , length(rval.length)
  , p(rval.p)
{
  rval.t = (size_t)Type::Null;
  rval.ownsContent = 0;
}

inline udVariant::udVariant(const udVariant &val)
  : t(val.t)
  , ownsContent(val.ownsContent)
  , length(val.length)
  , p(val.p)
{
  if (ownsContent)
  {
    if (is(Type::Component))
    {
      new((void*)&p) ud::ComponentRef((ud::ComponentRef&)val.c);
    }
    else if (is(Type::Delegate))
    {
      new((void*)&p) Delegate((Delegate&)val.p);
    }
    else if (is(Type::String))
    {
      char *pS = (char*)udAlloc(length);
      memcpy(pS, val.s, length);
      s = pS;
    }
    else if (is(Type::Array))
    {
      a = (udVariant*)udAlloc(sizeof(udVariant)*length);
      for (size_t i = 0; i<length; ++i)
        new((void*)&a[i]) udVariant(val.a[i]);
    }
    else if (is(Type::AssocArray))
    {
      aa = (udKeyValuePair*)udAlloc(sizeof(udKeyValuePair)*length);
      for (size_t i = 0; i<length; ++i)
      {
        new((void*)&aa[i].key) udVariant(val.aa[i].key);
        new((void*)&aa[i].value) udVariant(val.aa[i].value);
      }
    }
  }
}

inline udVariant::udVariant(bool b)
  : t((size_t)Type::Bool)
  , ownsContent(0)
  , length(0)
  , b(b)
{}
inline udVariant::udVariant(int64_t i)
  : t((size_t)Type::Int)
  , ownsContent(0)
  , length(0)
  , i(i)
{}
inline udVariant::udVariant(double f)
  : t((size_t)Type::Float)
  , ownsContent(0)
  , length(0)
  , f(f)
{}
inline udVariant::udVariant(ud::ComponentRef &&spC)
  : t((size_t)Type::Component)
  , ownsContent(1)
  , length(0)
{
  new(&p) ud::ComponentRef(std::move(spC));
}
inline udVariant::udVariant(const ud::ComponentRef &spC)
  : t((size_t)Type::Component)
  , ownsContent(1)
  , length(0)
{
  new(&p) ud::ComponentRef(spC);
}
inline udVariant::udVariant(const Delegate &d)
  : t((size_t)Type::Delegate)
  , ownsContent(1)
  , length(0)
{
  new(&p) Delegate(d);
}
inline udVariant::udVariant(Delegate &&d)
  : t((size_t)Type::Delegate)
  , ownsContent(1)
  , length(0)
{
  new(&p) Delegate(std::move(d));
}
inline udVariant::udVariant(udString s, bool ownsMemory)
  : t((size_t)Type::String)
  , ownsContent(ownsMemory ? 1 : 0)
  , length(s.length)
  , s(s.ptr)
{}
inline udVariant::udVariant(udSlice<udVariant> a, bool ownsMemory)
  : t((size_t)Type::Array)
  , ownsContent(ownsMemory ? 1 : 0)
  , length(a.length)
  , a(a.ptr)
{}
inline udVariant::udVariant(udSlice<udKeyValuePair> aa, bool ownsMemory)
  : t((size_t)Type::AssocArray)
  , ownsContent(ownsMemory ? 1 : 0)
  , length(aa.length)
  , aa(aa.ptr)
{}

inline udVariant& udVariant::operator=(udVariant &&rval)
{
  if (this != &rval)
  {
    this->~udVariant();

    t = rval.t;
    ownsContent = rval.ownsContent;
    length = rval.length;
    p = rval.p;

    rval.t = (size_t)Type::Null;
    rval.ownsContent = false;
  }
  return *this;
}

inline udVariant& udVariant::operator=(const udVariant &rval)
{
  if (this != &rval)
  {
    this->~udVariant();
    new(this) udVariant(rval);
  }
  return *this;
}

inline udVariant::Type udVariant::type() const
{
  return (Type)t;
}

inline bool udVariant::is(Type type) const
{
  return (Type)t == type;
}


// *************************************
// ** template construction machinery **
// *************************************

// horrible hack to facilitate partial specialisations (support all of the types!)
template<typename T>
struct udVariant_Construct
{
  UDFORCE_INLINE static udVariant construct(T &&rval)
  {
    return udVariant(udToVariant(std::move(rval)));
  }
  UDFORCE_INLINE static udVariant construct(const T &v)
  {
    return udVariant(udToVariant(v));
  }
};
template<typename T>
udVariant::udVariant(T &&rval)
  : udVariant(udVariant_Construct<typename std::remove_const<T>::type>::construct(std::move(rval)))
{}
template<typename T>
udVariant::udVariant(const T &v)
  : udVariant(udVariant_Construct<typename std::remove_const<T>::type>::construct(v))
{}
template<typename T>
udVariant::udVariant(T &v)
  : udVariant((const T&)v)
{}

// specialisations of udVariant_Construct for all the basic types
template<> struct udVariant_Construct <nullptr_t> { UDFORCE_INLINE static udVariant construct(nullptr_t)       { return udVariant(); } };
template<> struct udVariant_Construct <float>     { UDFORCE_INLINE static udVariant construct(float f)         { return udVariant((double)f); } };
template<> struct udVariant_Construct <int8_t>    { UDFORCE_INLINE static udVariant construct(int8_t i)        { return udVariant((int64_t)i); } };
template<> struct udVariant_Construct <uint8_t>   { UDFORCE_INLINE static udVariant construct(uint8_t i)       { return udVariant((int64_t)(uint64_t)i); } };
template<> struct udVariant_Construct <int16_t>   { UDFORCE_INLINE static udVariant construct(int16_t i)       { return udVariant((int64_t)i); } };
template<> struct udVariant_Construct <uint16_t>  { UDFORCE_INLINE static udVariant construct(uint16_t i)      { return udVariant((int64_t)(uint64_t)i); } };
template<> struct udVariant_Construct <int32_t>   { UDFORCE_INLINE static udVariant construct(int32_t i)       { return udVariant((int64_t)i); } };
template<> struct udVariant_Construct <uint32_t>  { UDFORCE_INLINE static udVariant construct(uint32_t i)      { return udVariant((int64_t)(uint64_t)i); } };
template<> struct udVariant_Construct <uint64_t>  { UDFORCE_INLINE static udVariant construct(uint64_t i)      { return udVariant((int64_t)i); } };
template<> struct udVariant_Construct <char*>     { UDFORCE_INLINE static udVariant construct(const char *s)   { return udVariant(udString(s)); } };
template<size_t N>
struct udVariant_Construct <char[N]>              { UDFORCE_INLINE static udVariant construct(const char s[N]) { return udVariant(udString(s, N-1)); } };

// for arrays
template<typename T>
UDFORCE_INLINE udVariant udToVariant(const udSlice<T> arr)
{
  udVariant r;
  udVariant *a = r.allocArray(arr.length);
  for (size_t i = 0; i<arr.length; ++i)
    new(&a[i]) udVariant(arr[i]);
  return r;
}

// for components
UDFORCE_INLINE udVariant udToVariant(ud::ComponentRef &&rval)
{
  return udVariant(std::move(rval));
}
UDFORCE_INLINE udVariant udToVariant(const ud::ComponentRef &c)
{
  return udVariant(c);
}

// vectors and matrices (require partial specialisation)
#include "udMath.h"

template<typename F>
UDFORCE_INLINE udVariant udToVariant(const udVector2<F> &v)
{
  udVariant r;
  udVariant *a = r.allocArray(2);
  new(&a[0]) udVariant(v.x);
  new(&a[1]) udVariant(v.y);
  return r;
}
template<typename F>
UDFORCE_INLINE udVariant udToVariant(const udVector3<F> &v)
{
  udVariant r;
  udVariant *a = r.allocArray(3);
  new(&a[0]) udVariant(v.x);
  new(&a[1]) udVariant(v.y);
  new(&a[2]) udVariant(v.z);
  return r;
}
template<typename F>
UDFORCE_INLINE udVariant udToVariant(const udVector4<F> &v)
{
  udVariant r;
  udVariant *a = r.allocArray(4);
  new(&a[0]) udVariant(v.x);
  new(&a[1]) udVariant(v.y);
  new(&a[2]) udVariant(v.z);
  new(&a[3]) udVariant(v.w);
  return r;
}
template<typename F>
UDFORCE_INLINE udVariant udToVariant(const udMatrix4x4<F> &m)
{
  udVariant r;
  udVariant *a = r.allocArray(16);
  for (size_t i = 0; i<16; ++i)
    new(&a[i]) udVariant(m.a[i]);
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
class VarDelegate<R(Args...)> : public udDelegateMemento
{
protected:
  template<typename T>
  friend class udSharedPtr;

  template<size_t ...S>
  UDFORCE_INLINE static udVariant callFuncHack(udSlice<udVariant> args, const udDelegate<R(Args...)> &d, Sequence<S...>)
  {
    return udVariant(d(args[S].as<typename std::remove_reference<Args>::type>()...));
  }

  udVariant to(udSlice<udVariant> args) const
  {
    // we need a call shim which can give an integer sequence as S... (Bjarne!!!)
    return callFuncHack(args, udDelegate<R(Args...)>(target), typename GenSequence<sizeof...(Args)>::type());
  }

  R from(Args... args) const
  {
    udVariant::Delegate d(target);

    // HAX: added 1 to support the case of zero args
    udVariant vargs[sizeof...(args)+1] = { udVariant(args)... };
    udSlice<udVariant> sargs(vargs, sizeof...(args));

    return d(sargs).as<R>();
  }

  // *to* udVariant::Delegate constructor
  VarDelegate(const udDelegate<R(Args...)> &d)
    : target(d.GetMemento())
  {
    FastDelegate<udVariant(udSlice<udVariant>)> shim(this, &VarDelegate::to);
    m = shim.GetMemento();
  }

  // *from* udVariant::Delegate constructor
  VarDelegate(const udVariant::Delegate &d)
    : target(d.GetMemento())
  {
    FastDelegate<R(Args...)> shim(this, &VarDelegate::from);
    m = shim.GetMemento();
  }

  const udDelegateMementoRef target;

private:
  VarDelegate<R(Args...)>& operator=(const VarDelegate<R(Args...)> &rh) = delete;
};

// specialise for 'void' return type
template<typename... Args>
class VarDelegate<void(Args...)> : public udDelegateMemento
{
protected:
  template<typename T>
  friend class udSharedPtr;

  template<size_t ...S>
  UDFORCE_INLINE static void callFuncHack(udSlice<udVariant> args, const udDelegate<void(Args...)> &d, Sequence<S...>)
  {
    d(args[S].as<typename std::remove_reference<Args>::type>()...);
  }

  udVariant to(udSlice<udVariant> args) const
  {
    // we need a call shim which can give an integer sequence as S... (Bjarne!!!)
    callFuncHack(args, udDelegate<void(Args...)>(target), typename GenSequence<sizeof...(Args)>::type());
    return udVariant();
  }

  void from(Args... args) const
  {
    udVariant::Delegate d(target);

    // HAX: added 1 to support the case of zero args
    udVariant vargs[sizeof...(args)+1] = { udVariant(args)... };
    udSlice<udVariant> sargs(vargs, sizeof...(args));

    d(sargs);
  }

  // *to* udVariant::Delegate constructor
  VarDelegate(const udDelegate<void(Args...)> &d)
    : target(d.GetMemento())
  {
    FastDelegate<udVariant(udSlice<udVariant>)> shim(this, &VarDelegate::to);
    m = shim.GetMemento();
  }

  // *from* udVariant::Delegate constructor
  VarDelegate(const udVariant::Delegate &d)
    : target(d.GetMemento())
  {
    FastDelegate<void(Args...)> shim(this, &VarDelegate::from);
    m = shim.GetMemento();
  }

  const udDelegateMementoRef target;

private:
  VarDelegate<void(Args...)>& operator=(const VarDelegate<void(Args...)> &rh) = delete;
};

template<typename R, typename... Args>
inline udVariant udToVariant(const udDelegate<R(Args...)> &d)
{
  typedef udSharedPtr<VarDelegate<R(Args...)>> VarDelegateRef;

  return udVariant::Delegate(VarDelegateRef::create(d));
}


// ********************************
// ** template casting machinery **
// ********************************

// HAX: this is a horrible hax to satisfy the C++ compiler!
template<typename T>
struct udVariant_Cast
{
  inline static T as(const udVariant &v) { T r; udFromVariant(v, &r); return r; }
};

template<typename T>
inline T udVariant::as() const
{
  // HACK: pipe this through a class so we can partial-specialise
  return udVariant_Cast<T>::as(*this);
}

// partial specialisation for const
template<typename T>
struct udVariant_Cast < const T >
{
  inline static const T as(const udVariant &v)
  {
    return udVariant_Cast<T>::as(v);
  }
};

// specialisations for udVeriant::as()
template<> struct udVariant_Cast < bool     > { inline static bool     as(const udVariant &v) { return v.asBool(); } };
template<> struct udVariant_Cast < float    > { inline static float    as(const udVariant &v) { return (float)v.asFloat(); } };
template<> struct udVariant_Cast < double   > { inline static double   as(const udVariant &v) { return v.asFloat(); } };
template<> struct udVariant_Cast < int8_t   > { inline static int8_t   as(const udVariant &v) { return (int8_t)v.asInt(); } };
template<> struct udVariant_Cast < uint8_t  > { inline static uint8_t  as(const udVariant &v) { return (uint8_t)v.asInt(); } };
template<> struct udVariant_Cast < int16_t  > { inline static int16_t  as(const udVariant &v) { return (int16_t)v.asInt(); } };
template<> struct udVariant_Cast < uint16_t > { inline static uint16_t as(const udVariant &v) { return (uint16_t)v.asInt(); } };
template<> struct udVariant_Cast < int32_t  > { inline static int32_t  as(const udVariant &v) { return (int32_t)v.asInt(); } };
template<> struct udVariant_Cast < uint32_t > { inline static uint32_t as(const udVariant &v) { return (uint32_t)v.asInt(); } };
template<> struct udVariant_Cast < int64_t  > { inline static int64_t  as(const udVariant &v) { return (int64_t)v.asInt(); } };
template<> struct udVariant_Cast < uint64_t > { inline static uint64_t as(const udVariant &v) { return (uint64_t)v.asInt(); } };
template<> struct udVariant_Cast < udString > { inline static udString as(const udVariant &v) { return v.asString(); } };

template<> struct udVariant_Cast < udVariant > { inline static udVariant as(const udVariant &v) { return v; } };

// udMath types
template<typename U>
inline void udFromVariant(const udVariant &v, udVector2<U> *pR)
{
  *pR = udVector2<U>::zero();
  if (v.is(udVariant::Type::Array))
  {
    auto a = v.asArray();
    if (a.length >= 2)
    {
      for (size_t i = 0; i < 2; ++i)
        ((U*)pR)[i] = (U)a[i].asFloat();
    }
  }
  else if (v.is(udVariant::Type::AssocArray))
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
inline void udFromVariant(const udVariant &v, udVector3<U> *pR)
{
  *pR = udVector3<U>::zero();
  if (v.is(udVariant::Type::Array))
  {
    auto a = v.asArray();
    if (a.length >= 3)
    {
      for (size_t i = 0; i < 3; ++i)
        ((U*)pR)[i] = (U)a[i].asFloat();
    }
  }
  else if (v.is(udVariant::Type::AssocArray))
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
inline void udFromVariant(const udVariant &v, udVector4<U> *pR)
{
  *pR = udVector4<U>::zero();
  if (v.is(udVariant::Type::Array))
  {
    auto a = v.asArray();
    if (a.length >= 4)
    {
      for (size_t i = 0; i < 4; ++i)
        ((U*)pR)[i] = (U)a[i].asFloat();
    }
  }
  else if (v.is(udVariant::Type::AssocArray))
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
inline void udFromVariant(const udVariant &v, udMatrix4x4<U> *pR)
{
  *pR = udMatrix4x4<U>::identity();
  if (v.is(udVariant::Type::Array))
  {
    auto a = v.asArray();
    if (a.length >= 16)
    {
      for (size_t i = 0; i < 16; ++i)
        ((U*)pR)[i] = (U)a[i].asFloat();
    }
  }
  else if (v.is(udVariant::Type::AssocArray))
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
inline void udFromVariant(const udVariant &v, udDelegate<R(Args...)> *pD)
{
  typedef udSharedPtr<VarDelegate<R(Args...)>> VarDelegateRef;

  if (v.asDelegate())
    *pD = udDelegate<R(Args...)>(VarDelegateRef::create(v.asDelegate()));
  else
    *pD = nullptr;
}
