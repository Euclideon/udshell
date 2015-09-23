#pragma once
#if !defined(_STRINGOF_H)
#define _STRINGOF_H

#include <type_traits>
#include <tuple>

template<typename T>
udString udStringify();

struct udVariant;

namespace ud {
namespace internal {

  // ***
  // *** ALL THESE SHOULD USE udMutableString<>
  // *** BUT WE NEED TO FINISH THE STRING FUNCTIONS
  // ***

  template<typename T, typename... Types>
  struct Stringify {
    static udSharedString stringify()
    {
      return ::udStringify<T>();
    }
  };

  template<typename T1, typename T2, typename... Types>
  struct Stringify<T1, T2, Types...> {
    static udSharedString stringify()
    {
      return udSharedString::concat(Stringify<T1>::stringify(), ",", Stringify<T2, Types...>::stringify());
    }
  };

  template<> struct Stringify<void>     { static udString stringify() { return "void"; } };
  template<> struct Stringify<double>   { static udString stringify() { return "f64"; } };
  template<> struct Stringify<float>    { static udString stringify() { return "f32"; } };
  template<> struct Stringify<uint8_t>  { static udString stringify() { return "u8"; } };
  template<> struct Stringify<int8_t>   { static udString stringify() { return "s8"; } };
  template<> struct Stringify<char>     { static udString stringify() { return "s8"; } };
  template<> struct Stringify<uint16_t> { static udString stringify() { return "u16"; } };
  template<> struct Stringify<int16_t>  { static udString stringify() { return "s16"; } };
  template<> struct Stringify<uint32_t> { static udString stringify() { return "u32"; } };
  template<> struct Stringify<int32_t>  { static udString stringify() { return "s32"; } };
  template<> struct Stringify<uint64_t> { static udString stringify() { return "u64"; } };
  template<> struct Stringify<int64_t>  { static udString stringify() { return "s64"; } };

  template<typename T>
  struct Stringify<T*>    { static udSharedString stringify() { return udSharedString::concat(Stringify<T>::stringify(), "*"); } };

  template<typename T, size_t N>
  struct Stringify<T[N]>  {
    static udSharedString stringify()
    {
      char buffer[16];
      udSprintf(buffer, sizeof(buffer), "%d", (int)N);
      return udSharedString::concat(Stringify<T>::stringify(), "[", buffer, "]");
    }
  };

  template<typename... T>
  struct Stringify<std::tuple<T...>>
  {
    static udSharedString stringify()
    {
      return udSharedString::concat("{", Stringify<typename std::remove_const<typename std::remove_reference<T>::type>::type...>::stringify(), "}");
    }
  };

} // namespace internal
} // namespace ud

template<typename... Types>
udSharedString stringof()
{
  return ud::internal::Stringify<typename std::remove_const<typename std::remove_reference<Types>::type>::type...>::stringify();
}

#endif // _STRINGOF_H
