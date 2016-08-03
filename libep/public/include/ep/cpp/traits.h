#pragma once
#if !defined(__EP_TRAITS)
#define __EP_TRAITS

#include "ep/c/foreach.h"

#define DECLVAL_T(T, N) std::declval<T>()
#define DECLTYPE_VALUE(Name) decltype(Name<T>(0))::value

#define METHOD_EXISTS(Name, Method, ...) \
  template <typename U> static auto Name(int) -> decltype(std::declval<U>().Method(FOR_EACH_COMMA_SEP(DECLVAL_T, __VA_ARGS__)), std::true_type()); \
  template <typename> static std::false_type Name(...);

#define METHOD_EXISTS_RETURN(Name, Method, ReturnType, ...) \
  template <typename U> static auto Name(int) -> decltype(std::enable_if<ep::internal::decay_equiv<decltype(std::declval<U>().Method(FOR_EACH_COMMA_SEP(DECLVAL_T, __VA_ARGS__))), ReturnType>::value, int>::type(), std::true_type()); \
  template <typename> static std::false_type Name(...);

namespace ep {

namespace internal {

  template <typename T, typename U>
  struct decay_equiv : std::is_same<typename std::decay<T>::type, U>::type {};

  template <typename T> class KeyTypeImpl;
  template <typename T> class IndexTypeImpl;

  template <typename T> class HasFrontImpl;
  template <typename T> class HasBackImpl;
  template <typename T> class RandomAccessibleImpl;

  template <typename T, bool hasFront, bool hasBack, bool hasAt> struct ElementTypeImpl;

  template <typename T> class HasSizeImpl;

  template <typename T> struct GrowableImpl;
  template <typename T> struct ShrinkableImpl;

} // namespace internal


template <bool b>
struct BoolType { using type = std::false_type; };
template <>
struct BoolType<true> { using type = std::true_type; };

template <bool A, bool... More>
struct And { static constexpr bool value = A && And<More...>::value; using type = typename BoolType<value>::type; };
template <bool A>
struct And<A> { static constexpr bool value = A; using type = typename BoolType<value>::type; };

template <bool A, bool... More>
struct Or { static constexpr bool value = A || Or<More...>::value; using type = typename BoolType<value>::type; };
template <bool A>
struct Or<A> { static constexpr bool value = A; using type = typename BoolType<value>::type; };


template <typename T>
using KeyType = typename internal::KeyTypeImpl<T>::type;
template <typename T>
using IndexType = typename internal::IndexTypeImpl<T>::type;

template <typename T>
using HasFront = typename internal::HasFrontImpl<T>::type;
template <typename T>
using HasBack = typename internal::HasBackImpl<T>::type;
template <typename T>
using RandomAccessible = typename internal::RandomAccessibleImpl<T>::type;

template <typename T>
using ElementType = typename internal::ElementTypeImpl<T, HasFront<T>::value, HasBack<T>::value, RandomAccessible<T>::value>::type;

template <typename T>
struct IsShared
{
  // refCount()
  // incRef(), decRef() TODO: <- ???
  METHOD_EXISTS_RETURN(Impl, use_count, size_t)
public:
  static constexpr bool value = DECLTYPE_VALUE(Impl);
  using type = typename BoolType<value>::type;
};

// container concepts:

template <typename T>
using HasSize = typename internal::HasSizeImpl<T>::type;

template <typename T>
using Growable = typename internal::GrowableImpl<T>::type;
template <typename T>
using Shrinkable = typename internal::ShrinkableImpl<T>::type;

template <typename T>
using IsContainer = typename HasSize<T>::type;

template <typename T>
using IsMutable = typename And<Growable<T>::value, Shrinkable<T>::value>::type;

#if 0 // TODO: consider testing with AVLTree::PredFunctor
template <typename T>
struct IsSorted
{
  T::IsSorted == true
};
#endif // 0

template <typename T>
struct IsKeyed
{
  // T::KeyValuePair, T::KeyType
  template <typename U> static auto Impl(int) -> decltype(std::declval<typename U::KeyValuePair>(), std::declval<typename U::KeyType>(), std::true_type());
  template <typename> static std::false_type Impl(...);
public:
  using type = decltype(Impl<T>(0));
  static constexpr bool value = type::value;
};


// range concepts:
template <typename T>
using IsInputRange = typename And<HasFront<T>::value, Shrinkable<T>::value>::type;
template <typename T>
using IsForwardRange = typename And<IsInputRange<T>::value, HasSize<T>::value>::type;
template <typename T>
using IsReverseRange = typename And<HasBack<T>::value, Shrinkable<T>::value, HasSize<T>::value>::type;
template <typename T>
using IsBidirectionalRange = typename And<IsForwardRange<T>::value, IsReverseRange<T>::value>::type;

} // namespace ep

#include "ep/cpp/internal/traits_inl.h"

#endif // __EP_TRAITS
