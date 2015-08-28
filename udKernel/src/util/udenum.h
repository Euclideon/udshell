#pragma once
#if !defined(_UDENUM_H)
#define _UDENUM_H

#include "udstring.h"
#include "util/foreach.h"
#include <type_traits>

#define STRINGIFY(a, i) #a,
#define SHIFT_LEFT(a, i) a = 1u<<(i),

struct udEnum {};
struct udBitfield {};

struct udEnumDesc
{
  udString name;
  udSlice<const udString> keys;
  void (*stringify)(size_t val, udFixedString64 &s);
};

#define UD_ENUM(NAME, ...)                                                        \
  struct NAME : public udEnum                                                     \
  {                                                                               \
    enum EnumKeys                                                                 \
    {                                                                             \
      Invalid = -1,                                                               \
      __VA_ARGS__                                                                 \
    };                                                                            \
    typedef std::underlying_type<EnumKeys>::type Type;                            \
                                                                                  \
    Type v;                                                                       \
                                                                                  \
    NAME() : v(Invalid) {}                                                        \
    NAME(const NAME &e) : v(e.v) {}                                               \
    NAME(Type v) : v(v) {}                                                        \
    NAME(udString s)                                                              \
    {                                                                             \
      udSlice<const udString> keys = Keys();                                      \
      for(size_t i = 0; i < keys.length; ++i)                                     \
      {                                                                           \
        if (keys.ptr[i].eq(s))                                                    \
        {                                                                         \
          v = (Type)i;                                                            \
          break;                                                                  \
        }                                                                         \
      }                                                                           \
    }                                                                             \
                                                                                  \
    operator Type() const { return v; }                                           \
                                                                                  \
    udString StringOf() const                                                     \
    {                                                                             \
      return v == -1 ? "Invalid" : Keys()[v];                                     \
    }                                                                             \
                                                                                  \
    static udString Name()                                                        \
    {                                                                             \
      static udString name(#NAME);                                                \
      return name;                                                                \
    }                                                                             \
    static udSlice<const udString> Keys()                                         \
    {                                                                             \
      static udSlice<const udString> keys = { FOR_EACH(STRINGIFY, __VA_ARGS__) }; \
      return keys;                                                                \
    }                                                                             \
    static const udEnumDesc* Desc()                                               \
    {                                                                             \
      static const udEnumDesc desc = { Name(), Keys(),                            \
        [](size_t v, udFixedString64 &s) { NAME e((Type)v); s = e.StringOf(); } };\
      return &desc;                                                               \
    }                                                                             \
  };                                                                              \
  inline NAME udGetValAsEnum(NAME::EnumKeys e)                                    \
  {                                                                               \
    return NAME(e);                                                               \
  }

#define UD_BITFIELD(NAME, ...)                                                    \
  struct NAME : public udBitfield                                                 \
  {                                                                               \
    enum EnumKeys                                                                 \
    {                                                                             \
      FOR_EACH(SHIFT_LEFT, __VA_ARGS__)                                           \
    };                                                                            \
    typedef std::underlying_type<EnumKeys>::type Type;                            \
                                                                                  \
    Type v;                                                                       \
                                                                                  \
    NAME() : v(0) {}                                                              \
    NAME(const NAME &e) : v(e.v) {}                                               \
    NAME(Type v) : v(v) {}                                                        \
    NAME(udString s)                                                              \
    {                                                                             \
      udSlice<const udString> keys = Keys();                                      \
      for(size_t i = 0; i < keys.length; ++i)                                     \
      {                                                                           \
        if (keys.ptr[i].eq(s))                                                    \
        {                                                                         \
          v = (Type)i;                                                            \
          break;                                                                  \
        }                                                                         \
      }                                                                           \
    }                                                                             \
                                                                                  \
    operator Type() const { return v; }                                           \
                                                                                  \
    NAME& operator |=(NAME rh) { v = v | rh.v; return *this; }                    \
    NAME& operator &=(NAME rh) { v = v & rh.v; return *this; }                    \
    NAME& operator ^=(NAME rh) { v = v ^ rh.v; return *this; }                    \
                                                                                  \
    udFixedString64 StringOf()                                                    \
    {                                                                             \
      udFixedString64 r;                                                          \
      for(size_t i = 0; i < 32; ++i)                                              \
      {                                                                           \
        if (v & (1<<i))                                                           \
        {                                                                         \
          if (r.length > 0)                                                       \
            r.concat("|");                                                        \
          r.concat(Keys()[i]);                                                    \
        }                                                                         \
      }                                                                           \
      return r;                                                                   \
    }                                                                             \
                                                                                  \
    static udString Name()                                                        \
    {                                                                             \
      static udString name = #NAME;                                               \
      return name;                                                                \
    }                                                                             \
    static udSlice<const udString> Keys()                                         \
    {                                                                             \
      static udSlice<const udString> keys = { FOR_EACH(STRINGIFY, __VA_ARGS__) }; \
      return keys;                                                                \
    }                                                                             \
    static const udEnumDesc* Desc()                                               \
    {                                                                             \
      static const udEnumDesc desc = { Name(), Keys(),                            \
        [](size_t v, udFixedString64 &s) { NAME e((Type)v); s = e.StringOf(); } };\
      return &desc;                                                               \
    }                                                                             \
  };                                                                              \
  inline NAME udGetValAsEnum(NAME::EnumKeys e)                                    \
  {                                                                               \
    return NAME(e);                                                               \
  }


#endif // _UDENUM_H
