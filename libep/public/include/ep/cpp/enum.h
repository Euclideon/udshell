#pragma once
#if !defined(_EPENUM_H)
#define _EPENUM_H

#include "ep/cpp/platform.h"
#include "ep/c/foreach.h"
#include <utility>


namespace ep {

struct Enum {};
struct Bitfield {};

struct EnumDesc
{
  String name;
  Slice<const String> keys;
  void (*stringify)(size_t val, MutableString64 &s);
};

} // namespace ep


#define STRINGIFY(a, i) #a,
#define SHIFT_LEFT(a, i) a = 1u<<(i),

#define EP_ENUM(NAME, ...)                                                        \
  struct NAME : public ep::Enum                                                   \
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
    NAME(Type _v) : v(_v) {}                                                      \
    NAME(ep::String s) : v(Invalid)                                               \
    {                                                                             \
      ep::Slice<const ep::String> keys = Keys();                                  \
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
    ep::String StringOf() const                                                   \
    {                                                                             \
      return v == -1 ? "Invalid" : Keys()[v];                                     \
    }                                                                             \
                                                                                  \
    static ep::String Name()                                                      \
    {                                                                             \
      static ep::String name(#NAME);                                              \
      return name;                                                                \
    }                                                                             \
    static ep::Slice<const ep::String> Keys()                                     \
    {                                                                             \
      using namespace ep;                                                         \
      static Array<const String> keys = { FOR_EACH(STRINGIFY, __VA_ARGS__) };     \
      return keys;                                                                \
    }                                                                             \
    static const ep::EnumDesc* Desc()                                             \
    {                                                                             \
      using namespace ep;                                                         \
      static const EnumDesc desc = { Name(), Keys(),                              \
        [](size_t _v, MutableString64 &s) { NAME e((Type)_v); s = e.StringOf(); } };\
      return &desc;                                                               \
    }                                                                             \
  };                                                                              \
  inline NAME epGetValAsEnum(NAME::EnumKeys e)                                    \
  {                                                                               \
    return NAME(e);                                                               \
  }

#define EP_BITFIELD(NAME, ...)                                                    \
  struct NAME : public ep::Bitfield                                               \
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
    NAME(Type _v) : v(_v) {}                                                      \
    NAME(ep::String s)                                                            \
    {                                                                             \
      ep::Slice<const ep::String> keys = Keys();                                  \
      for(size_t i = 0; i < keys.length; ++i)                                     \
      {                                                                           \
        if (keys.ptr[i].eq(s))                                                    \
        {                                                                         \
          v = 1<<i;                                                               \
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
    ep::MutableString64 StringOf() const                                          \
    {                                                                             \
      ep::MutableString64 r;                                                      \
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
    static ep::String Name()                                                      \
    {                                                                             \
      static ep::String name = #NAME;                                             \
      return name;                                                                \
    }                                                                             \
    static ep::Slice<const ep::String> Keys()                                     \
    {                                                                             \
      using namespace ep;                                                         \
      static Array<const String> keys = { FOR_EACH(STRINGIFY, __VA_ARGS__) };     \
      return keys;                                                                \
    }                                                                             \
    static const ep::EnumDesc* Desc()                                             \
    {                                                                             \
      using namespace ep;                                                         \
      static const EnumDesc desc = { Name(), Keys(),                              \
        [](size_t _v, MutableString64 &s) { NAME e((Type)_v); s = e.StringOf(); } };\
      return &desc;                                                               \
    }                                                                             \
  };                                                                              \
  inline NAME epGetValAsEnum(NAME::EnumKeys e)                                    \
  {                                                                               \
    return NAME(e);                                                               \
  }

#endif // _EPENUM_H
