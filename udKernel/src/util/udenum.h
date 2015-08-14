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

#define UD_ENUM(NAME, ...)                                                        \
  struct NAME : public udEnum                                                     \
  {                                                                               \
    enum EnumKeys                                                                 \
    {                                                                             \
      __VA_ARGS__                                                                 \
    };                                                                            \
    typedef std::underlying_type<EnumKeys>::type Type;                            \
                                                                                  \
    Type v;                                                                       \
                                                                                  \
    NAME() {}                                                                     \
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
      return Keys()[v];                                                           \
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
  };

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
    NAME() {}                                                                     \
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
    NAME operator |(NAME rh) const { NAME r; r.v = v | rh.v; return r; }          \
    NAME operator &(NAME rh) const { NAME r; r.v = v & rh.v; return r; }          \
    NAME operator ^(NAME rh) const { NAME r; r.v = v ^ rh.v; return r; }          \
    NAME operator ~() const        { NAME r; r.v = ~v; return r; }                \
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
  };

#endif // _UDENUM_H
