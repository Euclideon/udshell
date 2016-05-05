#pragma once
#if !defined(_STRINGOF_H)
#define _STRINGOF_H

#include "ep/cpp/platform.h"
#include "ep/cpp/math.h"

#include <utility>
#include <tuple>

template<typename T>
ep::String epStringify();

template<typename... Types>
ep::SharedString stringof();

namespace ep {

struct Variant;

namespace internal {

// ***
// *** ALL THESE SHOULD USE MutableString<>
// *** BUT WE NEED TO FINISH THE STRING FUNCTIONS
// ***

template<typename T, typename... Types>
struct Stringify {
  static SharedString stringify()
  {
    return ::epStringify<T>();
  }
};

template<typename T1, typename T2, typename... Types>
struct Stringify<T1, T2, Types...> {
  static SharedString stringify()
  {
    return SharedString::concat(Stringify<T1>::stringify(), ",", Stringify<T2, Types...>::stringify());
  }
};

template<> struct Stringify<bool>     { static String stringify() { return "bool"; } };
template<> struct Stringify<void>     { static String stringify() { return "void"; } };
template<> struct Stringify<double>   { static String stringify() { return "f64"; } };
template<> struct Stringify<float>    { static String stringify() { return "f32"; } };
template<> struct Stringify<uint8_t>  { static String stringify() { return "u8"; } };
template<> struct Stringify<int8_t>   { static String stringify() { return "s8"; } };
template<> struct Stringify<char>     { static String stringify() { return "s8"; } };
template<> struct Stringify<uint16_t> { static String stringify() { return "u16"; } };
template<> struct Stringify<int16_t>  { static String stringify() { return "s16"; } };
template<> struct Stringify<uint32_t> { static String stringify() { return "u32"; } };
template<> struct Stringify<int32_t>  { static String stringify() { return "s32"; } };
template<> struct Stringify<uint64_t> { static String stringify() { return "u64"; } };
template<> struct Stringify<int64_t>  { static String stringify() { return "s64"; } };

template <typename T> struct Stringify<Vector2<T>> { static SharedString stringify() { return SharedString(Concat, stringof<T>(), "[2]"); } };
template <typename T> struct Stringify<Vector3<T>> { static SharedString stringify() { return SharedString(Concat, stringof<T>(), "[3]"); } };
template <typename T> struct Stringify<Vector4<T>> { static SharedString stringify() { return SharedString(Concat, stringof<T>(), "[4]"); } };

template<typename T>
struct Stringify<T*>    { static SharedString stringify() { return SharedString::concat(Stringify<T>::stringify(), "*"); } };

template<typename T, size_t N>
struct Stringify<T[N]>  {
  static SharedString stringify()
  {
    return SharedString::concat(Stringify<T>::stringify(), "[", N, "]");
  }
};

template<typename... T>
struct Stringify<std::tuple<T...>>
{
  static SharedString stringify()
  {
    return SharedString::concat("{", Stringify<typename std::remove_const<typename std::remove_reference<T>::type>::type...>::stringify(), "}");
  }
};

} // namespace internal
} // namespace ep

template<typename... Types>
ep::SharedString stringof()
{
  return ep::internal::Stringify<typename std::remove_const<typename std::remove_reference<Types>::type>::type...>::stringify();
}

#endif // _STRINGOF_H
