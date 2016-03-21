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
template <typename T>
struct Range
{
  using ElementType = decltype(std::declval<T>().front());
  using Iterator = decltype(std::declval<T>().begin());

private:
  T range;

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
  template<typename... Args>
  Range(Args&&... args) : range(std::forward<Args>(args)...) {}

  static constexpr bool HasEmpty = decltype(TestHasEmpty<T>(0))::value;
  static constexpr bool HasLength = decltype(TestHasLength<T>(0))::value;
  static constexpr bool HasFront = decltype(TestHasFront<T>(0))::value;
  static constexpr bool HasBack = decltype(TestHasBack<T>(0))::value;
  static constexpr bool HasIndex = decltype(TestHasIndex<T>(0))::value;
  static constexpr bool HasFeatures = decltype(TestHasFeatures<T>(0))::value;

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
class VirtualRange
{
public:
  class Iterator
  {
    //...
  };

  VirtualRange() {}

  // TODO: copy construction should copy!!
  VirtualRange(const VirtualRange &rh) : spRangeInstance(rh.spRangeInstance) {}
  VirtualRange(VirtualRange &&rval) : spRangeInstance(std::move(rval.spRangeInstance)) {}

  template <typename R>
  VirtualRange(R &&srcRange)
  {
    class RangeType : public VirtualRangeImpl
    {
    public:
      RangeType(R &&srcRange) : range(std::forward<R>(srcRange)) {}

      Range<R> range;

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

  Iterator begin() const { EPASSERT(false, "TODO!"); }
  Iterator end() const { EPASSERT(false, "TODO!"); }

private:
  friend struct Variant;
  class VirtualRangeImpl : public RefCounted
  {
    template <typename T> friend class VirtualRange;

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

} // namespace ep

#include "ep/cpp/internal/range_inl.h"

#endif // _EPRANGE_HPP
