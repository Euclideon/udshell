#pragma once
#if !defined(_STRINGOF_H)
#define _STRINGOF_H

#include "ep/epplatform.h"

#include <type_traits>
#include <tuple>

template<typename T>
epString epStringify();

struct epVariant;

namespace ep {
namespace internal {

  // ***
  // *** ALL THESE SHOULD USE epMutableString<>
  // *** BUT WE NEED TO FINISH THE STRING FUNCTIONS
  // ***

  template<typename T, typename... Types>
  struct Stringify {
    static epSharedString stringify()
    {
      return ::epStringify<T>();
    }
  };

  template<typename T1, typename T2, typename... Types>
  struct Stringify<T1, T2, Types...> {
    static epSharedString stringify()
    {
      return epSharedString::concat(Stringify<T1>::stringify(), ",", Stringify<T2, Types...>::stringify());
    }
  };

  template<> struct Stringify<void>     { static epString stringify() { return "void"; } };
  template<> struct Stringify<double>   { static epString stringify() { return "f64"; } };
  template<> struct Stringify<float>    { static epString stringify() { return "f32"; } };
  template<> struct Stringify<uint8_t>  { static epString stringify() { return "u8"; } };
  template<> struct Stringify<int8_t>   { static epString stringify() { return "s8"; } };
  template<> struct Stringify<char>     { static epString stringify() { return "s8"; } };
  template<> struct Stringify<uint16_t> { static epString stringify() { return "u16"; } };
  template<> struct Stringify<int16_t>  { static epString stringify() { return "s16"; } };
  template<> struct Stringify<uint32_t> { static epString stringify() { return "u32"; } };
  template<> struct Stringify<int32_t>  { static epString stringify() { return "s32"; } };
  template<> struct Stringify<uint64_t> { static epString stringify() { return "u64"; } };
  template<> struct Stringify<int64_t>  { static epString stringify() { return "s64"; } };

  template<typename T>
  struct Stringify<T*>    { static epSharedString stringify() { return epSharedString::concat(Stringify<T>::stringify(), "*"); } };

  template<typename T, size_t N>
  struct Stringify<T[N]>  {
    static epSharedString stringify()
    {
      return epSharedString::concat(Stringify<T>::stringify(), "[", N, "]");
    }
  };

  template<typename... T>
  struct Stringify<std::tuple<T...>>
  {
    static epSharedString stringify()
    {
      return epSharedString::concat("{", Stringify<typename std::remove_const<typename std::remove_reference<T>::type>::type...>::stringify(), "}");
    }
  };

} // namespace internal
} // namespace ep

template<typename... Types>
epSharedString stringof()
{
  return ep::internal::Stringify<typename std::remove_const<typename std::remove_reference<Types>::type>::type...>::stringify();
}

#endif // _STRINGOF_H
