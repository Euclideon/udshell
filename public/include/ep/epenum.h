#pragma once
#if !defined(_EPENUM_H)
#define _EPENUM_H

#include "ep/epstring.h"
#include "ep/epforeach.h"
#include <type_traits>

#define STRINGIFY(a, i) #a,
#define SHIFT_LEFT(a, i) a = 1u<<(i),

struct epEnum {};
struct epBitfield {};

struct epEnumDesc
{
  epString name;
  epSlice<const epString> keys;
  void (*stringify)(size_t val, epMutableString64 &s);
};

#define EP_ENUM(NAME, ...)                                                        \
  struct NAME : public epEnum                                                     \
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
    NAME(epString s)                                                              \
    {                                                                             \
      epSlice<const epString> keys = Keys();                                      \
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
    epString StringOf() const                                                     \
    {                                                                             \
      return v == -1 ? "Invalid" : Keys()[v];                                     \
    }                                                                             \
                                                                                  \
    static epString Name()                                                        \
    {                                                                             \
      static epString name(#NAME);                                                \
      return name;                                                                \
    }                                                                             \
    static epSlice<const epString> Keys()                                         \
    {                                                                             \
      static epArray<const epString, 16> keys = { FOR_EACH(STRINGIFY, __VA_ARGS__) }; \
      return keys;                                                                \
    }                                                                             \
    static const epEnumDesc* Desc()                                               \
    {                                                                             \
      static const epEnumDesc desc = { Name(), Keys(),                            \
        [](size_t v, epMutableString64 &s) { NAME e((Type)v); s = e.StringOf(); } };\
      return &desc;                                                               \
    }                                                                             \
  };                                                                              \
  inline NAME epGetValAsEnum(NAME::EnumKeys e)                                    \
  {                                                                               \
    return NAME(e);                                                               \
  }

#define EP_BITFIELD(NAME, ...)                                                    \
  struct NAME : public epBitfield                                                 \
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
    NAME(epString s)                                                              \
    {                                                                             \
      epSlice<const epString> keys = Keys();                                      \
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
    epMutableString64 StringOf()                                                  \
    {                                                                             \
      epMutableString64 r;                                                        \
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
    static epString Name()                                                        \
    {                                                                             \
      static epString name = #NAME;                                               \
      return name;                                                                \
    }                                                                             \
    static epSlice<const epString> Keys()                                         \
    {                                                                             \
      static epArray<const epString, 16> keys = { FOR_EACH(STRINGIFY, __VA_ARGS__) }; \
      return keys;                                                                \
    }                                                                             \
    static const epEnumDesc* Desc()                                               \
    {                                                                             \
      static const epEnumDesc desc = { Name(), Keys(),                            \
        [](size_t v, epMutableString64 &s) { NAME e((Type)v); s = e.StringOf(); } };\
      return &desc;                                                               \
    }                                                                             \
  };                                                                              \
  inline NAME epGetValAsEnum(NAME::EnumKeys e)                                    \
  {                                                                               \
    return NAME(e);                                                               \
  }


#endif // _EPENUM_H
