
#include <type_traits>

// constructors...
inline udVariant::udVariant()
  : t(Type::Null)
  , ownsArray(0)
  , length(0)
{}

inline udVariant::udVariant(udVariant &&rval)
  : t(rval.t)
  , ownsArray(rval.ownsArray)
  , length(rval.length)
  , p(rval.p)
{
  rval.ownsArray = false;
}

inline udVariant::udVariant(const udVariant &val)
  : t(val.t)
  , ownsArray(val.ownsArray)
  , length(val.length)
  , p(val.p)
{
  if (ownsArray)
  {
    if (t == Type::Array)
    {
      a = (udVariant*)udAlloc(sizeof(udVariant)*length);
      for (size_t i = 0; i<length; ++i)
        new((void*)&a[i]) udVariant(val.a[i]);
    }
    else if (t == Type::AssocArray)
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
inline udVariant::udVariant(udVariant &rval)
  : udVariant((const udVariant&)rval)
{}


inline udVariant::udVariant(bool b)
  : t(Type::Bool)
  , ownsArray(0)
  , length(0)
  , b(b)
{}
inline udVariant::udVariant(int64_t i)
  : t(Type::Int)
  , ownsArray(0)
  , length(0)
  , i(i)
{}
inline udVariant::udVariant(double f)
  : t(Type::Float)
  , ownsArray(0)
  , length(0)
  , f(f)
{}
inline udVariant::udVariant(udComponent *c)
  : t(Type::Component)
  , ownsArray(0)
  , length(0)
  , c(c)
{}
inline udVariant::udVariant(udString s)
  : t(Type::String)
  , ownsArray(0)
  , length(s.length)
  , s(s.ptr)
{}
inline udVariant::udVariant(udSlice<udVariant> a, bool ownsMemory)
  : t(Type::Array)
  , ownsArray(ownsMemory ? 1 : 0)
  , length(a.length)
  , a(a.ptr)
{}
inline udVariant::udVariant(udSlice<udKeyValuePair> aa, bool ownsMemory)
  : t(Type::AssocArray)
  , ownsArray(ownsMemory ? 1 : 0)
  , length(aa.length)
  , aa(aa.ptr)
{}


// horrible hack to facilitate partial specialisations (support all of the types!)
template<typename T>
struct udVariant_Construct
{
  inline static udVariant construct(const T &v)
  {
    return udVariant(udToVariant(v));
  }
};
template<typename T>
udVariant::udVariant(T &v)
  : udVariant(udVariant_Construct<T>::construct(v))
{}
template<typename T>
udVariant::udVariant(const T &v)
  : udVariant(udVariant_Construct<T>::construct(v))
{}

// specialisations of udVariant_Construct for all the basic types
template<> struct udVariant_Construct <nullptr_t  > { inline static udVariant construct(nullptr_t)       { return udVariant(); } };
template<> struct udVariant_Construct <float      > { inline static udVariant construct(float f)         { return udVariant((double)f); } };
template<> struct udVariant_Construct <int8_t     > { inline static udVariant construct(int8_t i)        { return udVariant((int64_t)i); } };
template<> struct udVariant_Construct <uint8_t    > { inline static udVariant construct(uint8_t i)       { return udVariant((int64_t)(uint64_t)i); } };
template<> struct udVariant_Construct <int16_t    > { inline static udVariant construct(int16_t i)       { return udVariant((int64_t)i); } };
template<> struct udVariant_Construct <uint16_t   > { inline static udVariant construct(uint16_t i)      { return udVariant((int64_t)(uint64_t)i); } };
template<> struct udVariant_Construct <int32_t    > { inline static udVariant construct(int32_t i)       { return udVariant((int64_t)i); } };
template<> struct udVariant_Construct <uint32_t   > { inline static udVariant construct(uint32_t i)      { return udVariant((int64_t)(uint64_t)i); } };
template<> struct udVariant_Construct <uint64_t   > { inline static udVariant construct(uint64_t i)      { return udVariant((int64_t)i); } };
template<> struct udVariant_Construct <char*> { inline static udVariant construct(const char *s)   { return udVariant(udString(s)); } };
template<size_t N>
struct udVariant_Construct <char[N]>          { inline static udVariant construct(const char s[N]) { return udVariant(udString(s, N)); } };

// partial for udSharedPtr, which further specialises for udComponentRef (and derived types)
template<typename T>
struct udVariant_Construct<udSharedPtr<T>>
{
  template<typename C, typename std::enable_if<!std::is_base_of<udComponent, C>::value>::type* = nullptr> // O_O
  inline static udVariant constructPtr(udSharedPtr<C> p)           { return udToVariant(*p); } // udSharedPtr's that aren't components
  inline static udVariant constructPtr(udSharedPtr<udComponent> c) { return udVariant(c.ptr()); } // udComponents go this way

  inline static udVariant construct(udSharedPtr<T> c) { return constructPtr(c); } // shim to satisfy C++
};

// vectors and matrices (require partial specialisation)
template<typename F>
struct udVariant_Construct <udVector2<F>>
{
  inline static udVariant construct(const udVector2<F> &v)
  {
    udVariant r;
    udVariant *a = r.allocArray(2);
    new(&a[0]) udVariant(v.x);
    new(&a[1]) udVariant(v.y);
    return r;
  }
};
template<typename F>
struct udVariant_Construct <udVector3<F>>
{
  inline static udVariant construct(const udVector3<F> &v)
  {
    udVariant r;
    udVariant *a = r.allocArray(3);
    new(&a[0]) udVariant(v.x);
    new(&a[1]) udVariant(v.y);
    new(&a[3]) udVariant(v.z);
    return r;
  }
};
template<typename F>
struct udVariant_Construct <udVector4<F>>
{
  inline static udVariant construct(const udVector4<F> &v)
  {
    udVariant r;
    udVariant *a = r.allocArray(4);
    new(&a[0]) udVariant(v.x);
    new(&a[1]) udVariant(v.y);
    new(&a[3]) udVariant(v.z);
    new(&a[4]) udVariant(v.w);
    return r;
  }
};
template<typename F>
struct udVariant_Construct <udMatrix4x4<F>>
{
  inline static udVariant construct(const udMatrix4x4<F> &m)
  {
    udVariant r;
    udVariant *a = r.allocArray(16);
    for (size_t i = 0; i<16; ++i)
      new(&a[i]) udVariant(m.a[i]);
    return r;
  }
};

// destructor
inline udVariant::~udVariant()
{
  if (ownsArray && t >= Type::Array)
  {
    if (t == Type::Array)
    {
      for (size_t i = 0; i < length; ++i)
        a[i].~udVariant();
    }
    else if (t == Type::AssocArray)
    {
      for (size_t i = 0; i < length; ++i)
      {
        aa[i].key.~udVariant();
        aa[i].value.~udVariant();
      }
    }
    udFree(a);
  }
}

inline udVariant& udVariant::operator=(udVariant &&rval)
{
  this->~udVariant();

  t = rval.t;
  ownsArray = rval.ownsArray;
  length = rval.length;
  p = rval.p;

  rval.ownsArray = false;
  return *this;
}

inline udVariant& udVariant::operator=(const udVariant &rval)
{
  this->~udVariant();
  new(this) udVariant(rval);
  return *this;
}

inline udVariant::Type udVariant::type() const
{
  return Type(t);
}

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

// way to make casts for other component types?
template<> struct udVariant_Cast < udComponentRef > { inline static udComponentRef as(const udVariant &v) { return v.asComponent(); } };
// TODO: C++11 magic to make this shit work for all kinda fo components


// udMath types
template<typename U>
struct udVariant_Cast < udVector2<U> > {
  static udVector2<U> as(const udVariant &v)
  {
    udVector2<U> r = udVector2<U>::zero();
    // TODO: support Type::Array
    if (udVariant::Type::AssocArray)
    {
      auto aa = v.asAssocArraySeries();
      if (aa.length >= 2)
      {
        for (size_t i = 0; i < 2; ++i)
          ((U*)&r)[i] = aa[i].value.as<U>();
      }
    }
    return r;
  }
};
template<typename U>
struct udVariant_Cast < udVector3<U> > {
  static udVector3<U> as(const udVariant &v)
  {
    udVector3<U> r = udVector3<U>::zero();
    // TODO: support Type::Array
    if (udVariant::Type::AssocArray)
    {
      auto aa = v.asAssocArraySeries();
      if (aa.length >= 3)
      {
        for (size_t i = 0; i < 3; ++i)
          ((U*)&r)[i] = aa[i].value.as<U>();
      }
    }
    return r;
  }
};
template<typename U>
struct udVariant_Cast < udVector4<U> > {
  static udVector4<U> as(const udVariant &v)
  {
    udVector4<U> r = udVector4<U>::zero();
    // TODO: support Type::Array
    if (udVariant::Type::AssocArray)
    {
      auto aa = v.asAssocArraySeries();
      if (aa.length >= 4)
      {
        for (size_t i = 0; i < 4; ++i)
          ((U*)&r)[i] = aa[i].value.as<U>();
      }
    }
    return r;
  }
};
template<typename U>
struct udVariant_Cast < udMatrix4x4<U> > {
  static udMatrix4x4<U> as(const udVariant &v)
  {
    udMatrix4x4<U> r = udMatrix4x4<U>::identity();
    // TODO: support Type::Array
    if (udVariant::Type::AssocArray)
    {
      auto aa = v.asAssocArraySeries();
      if (aa.length >= 16)
      {
        for (size_t i = 0; i < 16; ++i)
          ((U*)&r)[i] = aa[i].value.as<U>();
      }
    }
    return r;
  }
};
