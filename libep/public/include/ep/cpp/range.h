#pragma once
#if !defined(_EPRANGE_HPP)
#define _EPRANGE_HPP

#include "ep/cpp/platform.h"
#include "ep/cpp/keyvaluepair.h"
#include "ep/cpp/sharedptr.h"
#include "ep/cpp/enum.h"

namespace ep {

EP_BITFIELD(RangeFeatures,
  HasLength,
  ForwardRange,
  ReverseRange,
  RandomRange
);

// Range<> wraps a duck-typed struct that looks like a range, but may not implement all range features
// The missing features are detected, reported, and function stubs implemented for missing features
template <typename R>
struct Range
{
  using ElementType = decltype(std::declval<R>().front());
  using Iterator = decltype(std::declval<R>().begin());

private:
  R range;

  // ***** HORRIBLE SFINAE MAGIC!!! *****
  template <typename U> static auto TestHasEmpty(int) -> decltype(std::declval<U>().empty(), std::true_type());
  template <typename> static std::false_type TestHasEmpty(...);
  template <typename U> static auto TestHasLength(int) -> decltype(std::declval<U>().length(), std::true_type());
  template <typename> static std::false_type TestHasLength(...);
  template <typename U> static auto TestHasFront(int) -> decltype(std::declval<U>().front(), std::declval<U>().popFront(), std::true_type());
  template <typename> static std::false_type TestHasFront(...);
  template <typename U> static auto TestHasBack(int) -> decltype(std::declval<U>().back(), std::declval<U>().popBack(), std::true_type());
  template <typename> static std::false_type TestHasBack(...);
  template <typename U> static auto TestHasIndex(int) -> decltype(std::declval<U>()[0], std::true_type());
  template <typename> static std::false_type TestHasIndex(...);
  template <typename U> static auto TestHasFeatures(int) -> decltype(std::declval<U>().features(), std::true_type());
  template <typename> static std::false_type TestHasFeatures(...);
  // ***** PHEW, WE SURVIVED! *****

public:
  Range(Range<R> &&rval) : range(std::move(rval.range)) {}
  Range(const Range<R> &rh) : range(rh.range) {}
  template <typename S> Range(Range<S> &&rval) : range(std::move(rval.range)) {}
  template <typename S> Range(const Range<S> &rh) : range(rh.range) {}

  template <typename... Args>
  Range(Args&&... args) : range(std::forward<Args>(args)...) {}

  static constexpr bool HasEmpty = decltype(TestHasEmpty<R>(0))::value;
  static constexpr bool HasLength = decltype(TestHasLength<R>(0))::value;
  static constexpr bool HasFront = decltype(TestHasFront<R>(0))::value;
  static constexpr bool HasBack = decltype(TestHasBack<R>(0))::value;
  static constexpr bool HasIndex = decltype(TestHasIndex<R>(0))::value;
  static constexpr bool HasFeatures = decltype(TestHasFeatures<R>(0))::value;

  RangeFeatures features() const;

  bool empty() const;
  size_t length() const;

  ElementType front() const;
  ElementType popFront();

  ElementType back() const;
  ElementType popBack();

  ElementType operator[](size_t index) const;

  // TODO: sfinae for these...
  Iterator begin() const;
  Iterator end() const;
};


// Wraps a range with a virtual interface, allowing for common access to any range type
template <typename V>
struct VirtualRange
{
public:
  class Iterator
  {
    Iterator(VirtualRange<V> *pR) : pR(pR) {}

    VirtualRange<V> *pR = nullptr;

    Iterator &operator++()
    {
      pR->popFront();
      if (pR->empty())
        pR = nullptr;
      return *this;
    }

    // TODO: HAX! this can only compare against 'end()'
    bool operator!=(Iterator rhs) { return pR != rhs.pR; }

    const V operator*() const { return pR->front(); }
    V operator*() { return pR->front(); }
  };

  VirtualRange() {}

  VirtualRange(VirtualRange &&rval) : spRangeInstance(std::move(rval.spRangeInstance)) {}
  VirtualRange(const VirtualRange &rh) : spRangeInstance(rh.spRangeInstance->clone()) {}

  template <typename R>
  VirtualRange(R &&srcRange)
  {
    class RangeType : public VirtualRangeImpl
    {
    public:
      Range<R> range;

      RangeType(R &&srcRange) : range(std::forward<R>(srcRange)) {}

      SharedPtr<VirtualRangeImpl> clone() const override final { return SharedPtr<RangeType>::create(*this); }

      RangeFeatures features() const override final { return range.features(); }

      bool empty() const override final { return range.empty(); }
      size_t length() const override final { return range.length(); }

      V front() const override final { return range.front(); }
      V popFront() override final { return range.popFront(); }

      V back() const override final { return range.back(); }
      V popBack() override final { return range.popBack(); }

      V operator[](size_t index) const override final { return range[index]; }
    };
    spRangeInstance = SharedPtr<RangeType>::create(std::forward<R>(srcRange));
  }

  RangeFeatures features() const { return spRangeInstance->features(); }

  bool empty() const { return spRangeInstance ? spRangeInstance->empty() : true; }
  size_t length() const { return spRangeInstance->length(); }

  V front() const { return spRangeInstance->front(); }
  V popFront() { return spRangeInstance->popFront(); }

  V back() const { return spRangeInstance->back(); }
  V popBack() { return spRangeInstance->popBack(); }

  V operator[](size_t index) const { return (*spRangeInstance)[index]; }

  Iterator begin() const { return Iterator(this); }
  Iterator end() const { return Iterator(nullptr); }

private:
  friend struct Variant;
  class VirtualRangeImpl : public RefCounted
  {
    template <typename T> friend struct VirtualRange;

    virtual SharedPtr<VirtualRangeImpl> clone() const = 0;

    virtual RangeFeatures features() const = 0;

    virtual bool empty() const = 0;
    virtual size_t length() const = 0;

    virtual V front() const = 0;
    virtual V popFront() = 0;

    virtual V back() const = 0;
    virtual V popBack() = 0;

    virtual V operator[](size_t epUnusedParam(index)) const = 0;
  };

  SharedPtr<VirtualRangeImpl> spRangeInstance;
};


// TODO: maybe put this in its own file?
template <typename Tree>
struct TreeRange
{
public:
  using TreeType = Tree;
  using ValueType = typename Tree::ValueType;
  using Iterator = typename Tree::Iterator;

  TreeRange()
    : i(nullptr), len(0) {}

  TreeRange(const TreeRange<Tree> &rh)
    : i(rh.i), len(rh.len)
  {}
  TreeRange(TreeRange<Tree> &&rval)
    : i(std::move(rval.i)), len(rval.len)
  {
    rval.len = 0;
  }

  TreeRange(const Tree &tree)
    : i(tree.begin()), len(tree.size())
  {}

  bool empty() const { return len == 0; }
  size_t length() const { return len; }

  ValueType& front() const
  {
    return (*i).value;
  }

  ValueType& popFront()
  {
    ValueType &t = (*i).value; ++i;
    --len;
    // TODO: put this back
    //    EPTHROW_IF(i == Tree::end() && len != 0, epR_OutOfBounds, "AVLTree length ");
    return t;
  }

  // iterators
  Iterator begin() const { return i; }
  Iterator end() const { return Tree::end(); }

private:
  Iterator i;
  size_t len;
};

} // namespace ep

#include "ep/cpp/internal/range_inl.h"

#endif // _EPRANGE_HPP
