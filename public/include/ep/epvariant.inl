
#include <type_traits>

ptrdiff_t epStringifyVariant(epSlice<char> buffer, epString format, const epVariant &var, const epVarArg*);

namespace ep {
namespace internal {

template<> struct StringifyProxy<epVariant>
{
  inline static ptrdiff_t stringify(epSlice<char> buffer, epString format, const void *pData, const epVarArg *pArgs) { return epStringifyVariant(buffer, format, *(epVariant*)pData, pArgs); }
  inline static int64_t intify(const void *pData) { return ((epVariant*)pData)->asInt(); }
};

} // namespace internal
} // namespace ep

// constructors...
inline epVariant::epVariant()
  : t((size_t)Type::Null)
  , ownsContent(0)
  , length(0)
{}

inline epVariant::epVariant(epVariant &&rval)
  : t(rval.t)
  , ownsContent(rval.ownsContent)
  , length(rval.length)
  , p(rval.p)
{
  rval.t = (size_t)Type::Null;
  rval.ownsContent = 0;
}

inline epVariant::epVariant(const epVariant &val)
  : t(val.t)
  , ownsContent(val.ownsContent)
  , length(val.length)
  , p(val.p)
{
  if (ownsContent)
  {
    if (is(Type::Component))
    {
      new((void*)&p) ep::ComponentRef((ep::ComponentRef&)val.c);
    }
    else if (is(Type::Delegate))
    {
      new((void*)&p) VarDelegate((VarDelegate&)val.p);
    }
    else if (is(Type::String))
    {
      char *pS = (char*)udAlloc(length);
      memcpy(pS, val.s, length);
      s = pS;
    }
    else if (is(Type::Array))
    {
      a = (epVariant*)udAlloc(sizeof(epVariant)*length);
      for (size_t i = 0; i<length; ++i)
        new((void*)&a[i]) epVariant((const epVariant&)val.a[i]);
    }
    else if (is(Type::AssocArray))
    {
      aa = (epKeyValuePair*)udAlloc(sizeof(epKeyValuePair)*length);
      for (size_t i = 0; i<length; ++i)
      {
        new((void*)&aa[i].key) epVariant((const epVariant&)val.aa[i].key);
        new((void*)&aa[i].value) epVariant((const epVariant&)val.aa[i].value);
      }
    }
  }
}

inline epVariant::epVariant(bool b)
  : t((size_t)Type::Bool)
  , ownsContent(0)
  , length(0)
  , b(b)
{}
inline epVariant::epVariant(int64_t i)
  : t((size_t)Type::Int)
  , ownsContent(0)
  , length(0)
  , i(i)
{}
inline epVariant::epVariant(double f)
  : t((size_t)Type::Float)
  , ownsContent(0)
  , length(0)
  , f(f)
{}
inline epVariant::epVariant(size_t val, const epEnumDesc *pDesc, bool isBitfield)
  : t(isBitfield ? (size_t)Type::Bitfield : (size_t)Type::Enum)
  , ownsContent(0)
  , length(val)
  , p((void*)pDesc)
{}
inline epVariant::epVariant(ep::ComponentRef &&spC)
  : t((size_t)Type::Component)
  , ownsContent(1)
  , length(0)
{
  new(&p) ep::ComponentRef(std::move(spC));
}
inline epVariant::epVariant(const ep::ComponentRef &spC)
  : t((size_t)Type::Component)
  , ownsContent(1)
  , length(0)
{
  new(&p) ep::ComponentRef(spC);
}
inline epVariant::epVariant(const VarDelegate &d)
  : t((size_t)Type::Delegate)
  , ownsContent(1)
  , length(0)
{
  new(&p) VarDelegate(d);
}
inline epVariant::epVariant(VarDelegate &&d)
  : t((size_t)Type::Delegate)
  , ownsContent(1)
  , length(0)
{
  new(&p) VarDelegate(std::move(d));
}
inline epVariant::epVariant(epString s, bool ownsMemory)
  : t((size_t)Type::String)
  , ownsContent(ownsMemory ? 1 : 0)
  , length(s.length)
  , s(s.ptr)
{}
inline epVariant::epVariant(epSlice<epVariant> a, bool ownsMemory)
  : t((size_t)Type::Array)
  , ownsContent(ownsMemory ? 1 : 0)
  , length(a.length)
  , a(a.ptr)
{}
inline epVariant::epVariant(epSlice<epKeyValuePair> aa, bool ownsMemory)
  : t((size_t)Type::AssocArray)
  , ownsContent(ownsMemory ? 1 : 0)
  , length(aa.length)
  , aa(aa.ptr)
{}

inline epVariant& epVariant::operator=(epVariant &&rval)
{
  if (this != &rval)
  {
    this->~epVariant();

    t = rval.t;
    ownsContent = rval.ownsContent;
    length = rval.length;
    p = rval.p;

    rval.t = (size_t)Type::Null;
    rval.ownsContent = false;
  }
  return *this;
}

inline epVariant& epVariant::operator=(const epVariant &rval)
{
  if (this != &rval)
  {
    this->~epVariant();
    new(this) epVariant(rval);
  }
  return *this;
}

inline epVariant::Type epVariant::type() const
{
  return (Type)t;
}

inline bool epVariant::is(Type type) const
{
  return (Type)t == type;
}


// ***************************************************
// ** template construction machinery for epVariant **
// **         HERE BE RADIOACTIVE DRAGONS!!         **
// ***************************************************

// horrible hack to facilitate partial specialisations (support all of the types!)
template<typename T>
struct epVariant_Construct
{
  epforceinline static epVariant construct(T &&rval)
  {
    return epToVariant(std::move(rval));
  }
  epforceinline static epVariant construct(const T &v)
  {
    return epToVariant(v);
  }
};

// These pipe through to `struct epVariant_Construct<>` to facilitate a bunch of partial specialisation madness
template<typename T>
epforceinline epVariant::epVariant(T &&rval)
  : epVariant(epVariant_Construct<typename std::remove_reference<T>::type>::construct(std::forward<T>(rval)))
{}

// specialisation of non-const epVariant, which annoyingly gets hooked by the T& constructor instead of the copy constructor
template<> struct epVariant_Construct<epVariant>  { epforceinline static epVariant construct(const epVariant &v) { return epVariant(v); } };

// ** suite of specialisations required to wrangle every conceivable combination of 'const'
template<typename T>
struct epVariant_Construct<const T>               { epforceinline static epVariant construct(const T &v) { return epVariant((T&)v); } };
template<typename T, size_t N>
struct epVariant_Construct<const T[N]>            { epforceinline static epVariant construct(const T v[N]) { return epVariant_Construct<T[N]>::construct(v); } };
template<typename T>
struct epVariant_Construct<const T *>             { epforceinline static epVariant construct(const T *v) { return epVariant((T*)v); } };

// specialisations for all the basic types
template<> struct epVariant_Construct <nullptr_t> { epforceinline static epVariant construct(nullptr_t)       { return epVariant(); } };
template<> struct epVariant_Construct <float>     { epforceinline static epVariant construct(float f)         { return epVariant((double)f); } };
template<> struct epVariant_Construct <int8_t>    { epforceinline static epVariant construct(int8_t i)        { return epVariant((int64_t)i); } };
template<> struct epVariant_Construct <uint8_t>   { epforceinline static epVariant construct(uint8_t i)       { return epVariant((int64_t)(uint64_t)i); } };
template<> struct epVariant_Construct <int16_t>   { epforceinline static epVariant construct(int16_t i)       { return epVariant((int64_t)i); } };
template<> struct epVariant_Construct <uint16_t>  { epforceinline static epVariant construct(uint16_t i)      { return epVariant((int64_t)(uint64_t)i); } };
template<> struct epVariant_Construct <int32_t>   { epforceinline static epVariant construct(int32_t i)       { return epVariant((int64_t)i); } };
template<> struct epVariant_Construct <uint32_t>  { epforceinline static epVariant construct(uint32_t i)      { return epVariant((int64_t)(uint64_t)i); } };
template<> struct epVariant_Construct <uint64_t>  { epforceinline static epVariant construct(uint64_t i)      { return epVariant((int64_t)i); } };
template<> struct epVariant_Construct <char*>     { epforceinline static epVariant construct(const char *s)   { return epVariant(epString(s)); } };
template<size_t N>
struct epVariant_Construct <char[N]>              { epforceinline static epVariant construct(const char s[N]) { return epVariant(epString(s, N-1)); } };
template<typename T, size_t N>
struct epVariant_Construct <T[N]>                 { epforceinline static epVariant construct(const T a[N])    { return epVariant(epSlice<T>(a, N)); } };

// ******************************************************
// ** Take a breath; you survived, now back to sanity! **
// ******************************************************

// enum & bitfield keys
template<typename T, typename std::enable_if<std::is_enum<T>::value>::type* = nullptr>
inline epVariant epToVariant(T e)
{
  return epToVariant(epGetValAsEnum(e));
}

// enums & bitfields
template<typename T,
  typename std::enable_if<
    std::is_base_of<epEnum, T>::value ||
    std::is_base_of<epBitfield, T>::value
  >::type* = nullptr>
inline epVariant epToVariant(T e)
{
  return epVariant(e.v, e.Desc(), std::is_base_of<epBitfield, T>::value);
}

// for arrays
template<typename T>
inline epVariant epToVariant(const epSlice<T> arr)
{
  epVariant r;
  epVariant *a = r.allocArray(arr.length);
  for (size_t i = 0; i<arr.length; ++i)
    new(&a[i]) epVariant(arr[i]);
  return r;
}

// for components
epforceinline epVariant epToVariant(ep::ComponentRef &&rval)
{
  return epVariant(std::move(rval));
}
epforceinline epVariant epToVariant(const ep::ComponentRef &c)
{
  return epVariant(c);
}

// vectors and matrices (require partial specialisation)
#include "udMath.h"

template<typename F>
inline epVariant epToVariant(const udVector2<F> &v)
{
  epVariant r;
  epVariant *a = r.allocArray(2);
  new(&a[0]) epVariant(v.x);
  new(&a[1]) epVariant(v.y);
  return r;
}
template<typename F>
inline epVariant epToVariant(const udVector3<F> &v)
{
  epVariant r;
  epVariant *a = r.allocArray(3);
  new(&a[0]) epVariant(v.x);
  new(&a[1]) epVariant(v.y);
  new(&a[2]) epVariant(v.z);
  return r;
}
template<typename F>
inline epVariant epToVariant(const udVector4<F> &v)
{
  epVariant r;
  epVariant *a = r.allocArray(4);
  new(&a[0]) epVariant(v.x);
  new(&a[1]) epVariant(v.y);
  new(&a[2]) epVariant(v.z);
  new(&a[3]) epVariant(v.w);
  return r;
}
template<typename F>
inline epVariant epToVariant(const udMatrix4x4<F> &m)
{
  epVariant r;
  epVariant *a = r.allocArray(16);
  for (size_t i = 0; i<16; ++i)
    new(&a[i]) epVariant(m.a[i]);
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
class VarDelegate<R(Args...)> : public epDelegateMemento
{
protected:
  template<typename T>
  friend class epSharedPtr;

  template<size_t ...S>
  epforceinline static epVariant callFuncHack(epSlice<epVariant> args, const epDelegate<R(Args...)> &d, Sequence<S...>)
  {
    return epVariant(d(args[S].as<typename std::remove_reference<Args>::type>()...));
  }

  epVariant to(epSlice<epVariant> args) const
  {
    // we need a call shim which can give an integer sequence as S... (Bjarne!!!)
    return callFuncHack(args, epDelegate<R(Args...)>(target), typename GenSequence<sizeof...(Args)>::type());
  }

  R from(Args... args) const
  {
    epVariant::VarDelegate d(target);

    // HAX: added 1 to support the case of zero args
    epVariant vargs[sizeof...(args)+1] = { epVariant(args)... };
    epSlice<epVariant> sargs(vargs, sizeof...(args));

    return d(sargs).as<R>();
  }

  // *to* epVariant::Delegate constructor
  VarDelegate(const epDelegate<R(Args...)> &d)
    : target(d.GetMemento())
  {
    FastDelegate<epVariant(epSlice<epVariant>)> shim(this, &VarDelegate::to);
    m = shim.GetMemento();
  }

  // *from* epVariant::Delegate constructor
  VarDelegate(const epVariant::VarDelegate &d)
    : target(d.GetMemento())
  {
    FastDelegate<R(Args...)> shim(this, &VarDelegate::from);
    m = shim.GetMemento();
  }

  const epDelegateMementoRef target;

private:
  VarDelegate<R(Args...)>& operator=(const VarDelegate<R(Args...)> &rh) = delete;
};

// specialise for 'void' return type
template<typename... Args>
class VarDelegate<void(Args...)> : public epDelegateMemento
{
protected:
  template<typename T>
  friend class epSharedPtr;

  template<size_t ...S>
  epforceinline static void callFuncHack(epSlice<epVariant> args, const epDelegate<void(Args...)> &d, Sequence<S...>)
  {
    d(args[S].as<typename std::remove_reference<Args>::type>()...);
  }

  epVariant to(epSlice<epVariant> args) const
  {
    // we need a call shim which can give an integer sequence as S... (Bjarne!!!)
    callFuncHack(args, epDelegate<void(Args...)>(target), typename GenSequence<sizeof...(Args)>::type());
    return epVariant();
  }

  void from(Args... args) const
  {
    epVariant::VarDelegate d(target);

    // HAX: added 1 to support the case of zero args
    epVariant vargs[sizeof...(args)+1] = { epVariant(args)... };
    epSlice<epVariant> sargs(vargs, sizeof...(args));

    d(sargs);
  }

  // *to* epVariant::Delegate constructor
  VarDelegate(const epDelegate<void(Args...)> &d)
    : target(d.GetMemento())
  {
    FastDelegate<epVariant(epSlice<epVariant>)> shim(this, &VarDelegate::to);
    m = shim.GetMemento();
  }

  // *from* epVariant::Delegate constructor
  VarDelegate(const epVariant::VarDelegate &d)
    : target(d.GetMemento())
  {
    FastDelegate<void(Args...)> shim(this, &VarDelegate::from);
    m = shim.GetMemento();
  }

  const epDelegateMementoRef target;

private:
  VarDelegate<void(Args...)>& operator=(const VarDelegate<void(Args...)> &rh) = delete;
};

template<typename R, typename... Args>
inline epVariant epToVariant(const epDelegate<R(Args...)> &d)
{
  typedef epSharedPtr<VarDelegate<R(Args...)>> VarDelegateRef;

  return epVariant::VarDelegate(VarDelegateRef::create(d));
}


// ********************************
// ** template casting machinery **
// ********************************

// HAX: this is a horrible hax to satisfy the C++ compiler!
template<typename T>
struct epVariant_Cast
{
  inline static T as(const epVariant &v) { T r; epFromVariant(v, &r); return r; }
};

template<typename T>
inline T epVariant::as() const
{
  // HACK: pipe this through a class so we can partial-specialise
  return epVariant_Cast<T>::as(*this);
}

// partial specialisation for const
template<typename T>
struct epVariant_Cast < const T >
{
  inline static const T as(const epVariant &v)
  {
    return epVariant_Cast<T>::as(v);
  }
};

// specialisations for udVeriant::as()
template<> struct epVariant_Cast < bool     > { inline static bool     as(const epVariant &v) { return v.asBool(); } };
template<> struct epVariant_Cast < float    > { inline static float    as(const epVariant &v) { return (float)v.asFloat(); } };
template<> struct epVariant_Cast < double   > { inline static double   as(const epVariant &v) { return v.asFloat(); } };
template<> struct epVariant_Cast < int8_t   > { inline static int8_t   as(const epVariant &v) { return (int8_t)v.asInt(); } };
template<> struct epVariant_Cast < uint8_t  > { inline static uint8_t  as(const epVariant &v) { return (uint8_t)v.asInt(); } };
template<> struct epVariant_Cast < int16_t  > { inline static int16_t  as(const epVariant &v) { return (int16_t)v.asInt(); } };
template<> struct epVariant_Cast < uint16_t > { inline static uint16_t as(const epVariant &v) { return (uint16_t)v.asInt(); } };
template<> struct epVariant_Cast < int32_t  > { inline static int32_t  as(const epVariant &v) { return (int32_t)v.asInt(); } };
template<> struct epVariant_Cast < uint32_t > { inline static uint32_t as(const epVariant &v) { return (uint32_t)v.asInt(); } };
template<> struct epVariant_Cast < int64_t  > { inline static int64_t  as(const epVariant &v) { return (int64_t)v.asInt(); } };
template<> struct epVariant_Cast < uint64_t > { inline static uint64_t as(const epVariant &v) { return (uint64_t)v.asInt(); } };
template<> struct epVariant_Cast < epString > { inline static epString as(const epVariant &v) { return v.asString(); } };

template<> struct epVariant_Cast < epVariant > { inline static epVariant as(const epVariant &v) { return v; } };

// enums & bitfields
template<typename T,
  typename std::enable_if<
    std::is_base_of<epEnum, T>::value ||
    std::is_base_of<epBitfield, T>::value
  >::type* = nullptr>
inline void epFromVariant(const epVariant &v, T *pE)
{
  if (v.is(std::is_base_of<epBitfield, T>::value ? epVariant::Type::Bitfield : epVariant::Type::Enum))
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
  else if (v.is(epVariant::Type::Int))
    *pE = T((typename T::Type)v.asInt());
  else if (v.is(epVariant::Type::String))
    *pE = T(v.asString());
}

// udMath types
template<typename U>
inline void epFromVariant(const epVariant &v, udVector2<U> *pR)
{
  *pR = udVector2<U>::zero();
  if (v.is(epVariant::Type::Array))
  {
    auto a = v.asArray();
    if (a.length >= 2)
    {
      for (size_t i = 0; i < 2; ++i)
        ((U*)pR)[i] = (U)a[i].asFloat();
    }
  }
  else if (v.is(epVariant::Type::AssocArray))
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
inline void epFromVariant(const epVariant &v, udVector3<U> *pR)
{
  *pR = udVector3<U>::zero();
  if (v.is(epVariant::Type::Array))
  {
    auto a = v.asArray();
    if (a.length >= 3)
    {
      for (size_t i = 0; i < 3; ++i)
        ((U*)pR)[i] = (U)a[i].asFloat();
    }
  }
  else if (v.is(epVariant::Type::AssocArray))
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
inline void epFromVariant(const epVariant &v, udVector4<U> *pR)
{
  *pR = udVector4<U>::zero();
  if (v.is(epVariant::Type::Array))
  {
    auto a = v.asArray();
    if (a.length >= 4)
    {
      for (size_t i = 0; i < 4; ++i)
        ((U*)pR)[i] = (U)a[i].asFloat();
    }
  }
  else if (v.is(epVariant::Type::AssocArray))
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
inline void epFromVariant(const epVariant &v, udMatrix4x4<U> *pR)
{
  *pR = udMatrix4x4<U>::identity();
  if (v.is(epVariant::Type::Array))
  {
    auto a = v.asArray();
    if (a.length >= 16)
    {
      for (size_t i = 0; i < 16; ++i)
        ((U*)pR)[i] = (U)a[i].asFloat();
    }
  }
  else if (v.is(epVariant::Type::AssocArray))
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
inline void epFromVariant(const epVariant &v, epDelegate<R(Args...)> *pD)
{
  typedef epSharedPtr<VarDelegate<R(Args...)>> VarDelegateRef;

  if (v.asDelegate())
    *pD = epDelegate<R(Args...)>(VarDelegateRef::create(v.asDelegate()));
  else
    *pD = nullptr;
}
