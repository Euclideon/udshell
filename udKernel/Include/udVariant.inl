
inline udVariant::udVariant()
  : t(Type::Null)
  , ownsArray(0)
  , length(0)
{}
inline udVariant::udVariant(nullptr_t)
  : t(Type::Null)
  , ownsArray(0)
  , length(0)
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
inline udVariant::udVariant(udComponentRef &_c)
  : t(Type::Component)
  , ownsArray(0)
  , length(0)
  , c(_c.ptr())
{}
inline udVariant::udVariant(udString s)
  : t(Type::String)
  , ownsArray(0)
  , length(s.length)
  , s(s.ptr)
{}
inline udVariant::udVariant(udSlice<udVariant> a)
  : t(Type::Array)
  , ownsArray(0)
  , length(a.length)
  , a(a.ptr)
{}
inline udVariant::udVariant(udSlice<udKeyValuePair> aa)
  : t(Type::AssocArray)
  , ownsArray(0)
  , length(aa.length)
  , aa(aa.ptr)
{}

// math types
template<typename U>
udVariant::udVariant(const udVector2<U> &v)
  : t(Type::Array)
  , ownsArray(1)
  , length(2)
{
  a = (udVariant*)udAlloc(sizeof(udVariant)*length);
  new(&a[0]) udVariant(v.x);
  new(&a[1]) udVariant(v.y);
}
template<typename U>
udVariant::udVariant(const udVector3<U> &v)
  : t(Type::Array)
  , ownsArray(1)
  , length(3)
{
  a = (udVariant*)udAlloc(sizeof(udVariant)*length);
  new(&a[0]) udVariant(v.x);
  new(&a[1]) udVariant(v.y);
  new(&a[2]) udVariant(v.z);
}
template<typename U>
udVariant::udVariant(const udVector4<U> &v)
  : t(Type::Array)
  , ownsArray(1)
  , length(4)
{
  a = (udVariant*)udAlloc(sizeof(udVariant)*length);
  new(&a[0]) udVariant(v.x);
  new(&a[1]) udVariant(v.y);
  new(&a[2]) udVariant(v.z);
  new(&a[3]) udVariant(v.w);
}
template<typename U>
udVariant::udVariant(const udMatrix4x4<U> &m)
  : t(Type::Array)
  , ownsArray(1)
  , length(16)
{
  a = (udVariant*)udAlloc(sizeof(udVariant)*length);
  for (size_t i = 0; i<16; ++i)
    new(&a[i]) udVariant(m.a[i]);
}

inline udVariant::~udVariant()
{
  if (t >= Type::Array && ownsArray)
    udFree(a);
}

inline udVariant::Type udVariant::type() const
{
  return Type(t);
}

// HAX: this is a horrible hax to satisfy the C++ compiler!
template<typename T>
struct udVariant_Cast
{
  inline static T as(const udVariant &v);
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
