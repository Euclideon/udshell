namespace ep {
namespace internal {

  template <typename T>
  class KeyTypeImpl
  {
    template <typename U> static auto atParam(int) -> typename U::KeyType;
    template <typename U> static auto atParam(long) -> typename U::key_type;
  public:
    using type = typename std::decay<decltype(atParam<T>(0))>::type;
  };

  template <typename T>
  class IndexTypeImpl
  {
    template <typename U> static auto atParam(int) -> typename U::KeyType;
    template <typename U> static auto atParam(long) -> typename U::key_type;
    template <typename> static size_t atParam(...);
  public:
    using type = typename std::decay<decltype(atParam<T>(0))>::type;
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
    // at(K), decltype(at(K)) == T::ElementType, operator[](K)
    METHOD_EXISTS(A, at, ep::IndexType<T>)
    METHOD_EXISTS(B, operator[], ep::IndexType<T>)
  public:
    static constexpr bool value = And<DECLTYPE_VALUE(A), DECLTYPE_VALUE(B)>::value;
    using type = typename BoolType<value>::type;
  };

  template <typename T, bool hasFront, bool hasBack, bool hasAt>
  struct ElementTypeImpl // false, false, false
  {
    static_assert(sizeof(T) != 0, "Container has does not provide front(), back() or at()");
  };
  template <typename T>
  struct ElementTypeImpl<T, true, false, false>
  {
    using type = typename std::decay<decltype(std::declval<T>().front())>::type;
  };
  template <typename T>
  struct ElementTypeImpl<T, false, true, false>
  {
    using type = typename std::decay<decltype(std::declval<T>().back())>::type;
  };
  template <typename T>
  struct ElementTypeImpl<T, false, false, true>
  {
    using type = typename std::decay<decltype(std::declval<T>().at(std::declval<IndexType<T>>()))>::type;
  };
  template <typename T>
  struct ElementTypeImpl<T, true, true, false>
  {
    using type = typename std::decay<decltype(std::declval<T>().front())>::type;
  private:
    using back = typename std::decay<decltype(std::declval<T>().back())>::type;
    static_assert(std::is_same<type, back>::value, "front() and back() return different types");
  };
  template <typename T>
  struct ElementTypeImpl<T, true, false, true>
  {
    using type = typename std::decay<decltype(std::declval<T>().front())>::type;
  private:
    using at = typename std::decay<decltype(std::declval<T>().at(std::declval<IndexType<T>>()))>::type;
    static_assert(std::is_same<type, at>::value, "front() and at() return different types");
  };
  template <typename T>
  struct ElementTypeImpl<T, false, true, true>
  {
    using type = typename std::decay<decltype(std::declval<T>().back())>::type;
  private:
    using at = typename std::decay<decltype(std::declval<T>().at(std::declval<IndexType<T>>()))>::type;
    static_assert(std::is_same<type, at>::value, "back() and at() return different types");
  };
  template <typename T>
  struct ElementTypeImpl<T, true, true, true>
  {
    using type = typename std::decay<decltype(std::declval<T>().front())>::type;
  private:
    using back = typename std::decay<decltype(std::declval<T>().back())>::type;
    using at = typename std::decay<decltype(std::declval<T>().at(std::declval<IndexType<T>>()))>::type;
    static_assert(std::is_same<type, back>::value && std::is_same<type, at>::value, "front(), back() and at() don't return the same type");
  };

  template <typename T>
  class HasSizeImpl
  {
    // T::ElementType, empty(), size(), *** range() ***?
    METHOD_EXISTS(A, size)
    METHOD_EXISTS(B, empty)
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
    METHOD_EXISTS(A, push_front, ElementType<U>)
    METHOD_EXISTS(B, push_back, ElementType<U>)
    METHOD_EXISTS(C, insert, KeyType<U>, ElementType<U>)
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
    METHOD_EXISTS(A, pop_front)
    METHOD_EXISTS(B, pop_back)
    METHOD_EXISTS(C, remove, KeyType<U>)
  public:
    static constexpr bool value = Or<DECLTYPE_VALUE(A), DECLTYPE_VALUE(B), DECLTYPE_VALUE(C)>::value;
    using type = typename BoolType<value>::type;
  };

} // namespace internal
} // namespace ep
