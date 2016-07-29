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

template <bool A, bool... More>
struct Xor { static constexpr bool value = A ^^ Xor<More...>::value; using type = typename BoolType<value>::type; };
template <bool A>
struct Xor<A> { static constexpr bool value = A; using type = typename BoolType<value>::type; };

namespace internal {

  template <typename T, typename U>
  struct decay_equiv : std::is_same<typename std::decay<T>::type, U>::type {};

  template <typename T>
  class IsContainerImpl
  {
    // T::ElementType, empty(), size(), *** range() ***?
    METHOD_EXISTS(A, size)
    METHOD_EXISTS(B, empty)
  public:
    static constexpr bool value = And<DECLTYPE_VALUE(A), DECLTYPE_VALUE(B)>::value;
    using type = typename BoolType<value>::type;
  };

  template <typename T>
  class HasFrontImpl
  {
    // front()
    METHOD_EXISTS(Impl, front)
  public:
    static constexpr bool value = DECLTYPE_VALUE(Impl);
    using type = typename BoolType<value>::type;
  };
  template <typename T>
  class HasBackImpl
  {
    // back()
    METHOD_EXISTS(Impl, back)
  public:
    static constexpr bool value = DECLTYPE_VALUE(Impl);
    using type = typename BoolType<value>::type;
  };
  template <typename T>
  class RandomAccessibleImpl
  {
    // get(K), decltype(get(K)) == T::ElementType, operator[](K)
    METHOD_EXISTS(A, get, typename T::KeyType)
    METHOD_EXISTS(B, operator[], typename T::KeyType)
  public:
    static constexpr bool value = And<DECLTYPE_VALUE(A), DECLTYPE_VALUE(B)>::value;
    using type = typename BoolType<value>::type;
  };

  template <typename T>
  struct GrowableImpl
  {
    // HasFront => pushFront(E)
    // HasBack => pushBack(E)
    // RandomAccessible => insert(K, E)
    METHOD_EXISTS(A, pushFront, ElementType<U>)
    METHOD_EXISTS(B, pushBack, ElementType<U>)
    METHOD_EXISTS(C, insert, typename U::KeyType, ElementType<U>)
  public:
    static constexpr bool value = Or<DECLTYPE_VALUE(A), DECLTYPE_VALUE(B), DECLTYPE_VALUE(C)>::value;
    using type = typename BoolType<value>::type;
  };
  template <typename T>
  struct ShrinkableImpl
  {
    // HasFront => popFront()
    // HasBack => popBack()
    // RandomAccessible => remove(K)
    METHOD_EXISTS(A, popFront)
    METHOD_EXISTS(B, popBack)
    METHOD_EXISTS(C, remove, typename U::KeyType)
  public:
    static constexpr bool value = Or<DECLTYPE_VALUE(A), DECLTYPE_VALUE(B), DECLTYPE_VALUE(C)>::value;
    using type = typename BoolType<value>::type;
  };

} // namespace internal


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
using ElementType = typename std::decay<decltype(std::declval<T>().front())>::type; // TODO: more work!

template <typename T>
using IsContainer = typename internal::IsContainerImpl<T>::type;

template <typename T>
using HasFront = typename internal::HasFrontImpl<T>::type;
template <typename T>
using HasBack = typename internal::HasBackImpl<T>::type;
template <typename T>
using RandomAccessible = typename internal::RandomAccessibleImpl<T>::type;

template <typename T>
using Growable = typename internal::GrowableImpl<T>::type;
template <typename T>
using Shrinkable = typename internal::ShrinkableImpl<T>::type;

template <typename T>
using IsMutable = typename And<Growable<T>::value, Shrinkable<T>::value>::type;

template <typename T>
struct IsSorted
{
  // T::IsSorted == true
};

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


// ordered           slice, array, sharedarray, string, mutablestring, sharedstring
// sorted            avltree, sharedmap<avltree>
// refcounted        refcounted, sharedarray, sharedstring, sharedmap
// keyed             sharedmap<>, avltree, hashmap



// range concepts:

// finite
// forward
// reverse
// bidirectional
// randomaccessible

} // namespace ep

#endif // __EP_TRAITS
