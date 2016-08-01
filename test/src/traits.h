#pragma once
#if !defined(_EP_TEST_TRAITS_H)
#define _EP_TEST_TRAITS_H

#include "ep/cpp/traits.h"
#include "ep/cpp/string.h"
#include "ep/cpp/keyvaluepair.h"


// ------------------------------- Helpers ----------------------------------------------------

#define DEFINE_TEST_CONTAINER(Container)                                \
  namespace testTraits {                                                \
  struct Types {                                                        \
    using DefaultType = typename Container<int>;                        \
    using HoldsInt = typename Container<int>;                           \
    using HoldsFloat = typename Container<float>;                       \
    using HoldsDouble = typename Container<double>;                     \
    using HoldsBool = typename Container<bool>;                         \
    using HoldsChar = typename Container<char>;                         \
    using HoldsString = typename Container<ep::SharedString>;           \
  };                                                                    \
  }

#define DEFINE_TEST_CONTAINER_KEYED(Container, KeyType)                 \
  namespace testTraits {                                                \
  struct Types {                                                        \
    using DefaultType = typename Container<KeyType, int>;               \
    using HoldsInt = typename Container<KeyType, int>;                  \
    using HoldsFloat = typename Container<KeyType, float>;              \
    using HoldsDouble = typename Container<KeyType, double>;            \
    using HoldsBool = typename Container<KeyType, bool>;                \
    using HoldsChar = typename Container<KeyType, char>;                \
    using HoldsString = typename Container<KeyType, ep::SharedString>;  \
  };                                                                    \
  }


namespace testTraits {

// Value ...
template <typename Container, typename Value, typename std::enable_if<ep::IsKeyed<Container>::value>::type* = nullptr>
auto value(Value &&val)
{
  return Container::KeyValuePair(ep::SharedString::format("test_{0}", val), val);
}

template <typename Container, typename Value, typename std::enable_if<!ep::IsKeyed<Container>::value>::type* = nullptr>
Value value(Value &&val)
{
  return val;
}


// Add Item ...
template <typename Container, typename Item, typename std::enable_if<ep::HasFront<Container>::value && ep::Growable<Container>::value>::type* = nullptr>
void addItem(Container &container, Item &&item)
{
  container.pushFront(std::move(value<Container>(item)));
}

template <typename Container, typename Item, typename std::enable_if<ep::RandomAccessible<Container>::value && ep::Growable<Container>::value>::type* = nullptr>
void addItem(Container &container, Item &&item)
{
  container.insert(std::move(value<Container>(item)));
}

template <typename Container, typename Item, typename std::enable_if<!ep::Growable<Container>::value>::type* = nullptr>
void addItem(Container &container, Item &&item)
{
  EPASSERT(false, "Called testTraits::addItem(...) on a non growable container");
}

} // namespace traits



// ------------------------------- HasSize Trait ----------------------------------------------

template <typename T>
class Traits_HasSizeTest : public ::testing::Test { };
TYPED_TEST_CASE_P(Traits_HasSizeTest);


TYPED_TEST_P(Traits_HasSizeTest, SizeAndEmpty)
{
  // Verify we have the trait
  EXPECT_TRUE(ep::HasSize<TypeParam::HoldsInt>::value);

  // Default container is empty
  TypeParam::HoldsInt container;
  EXPECT_EQ(0, container.size());
  EXPECT_TRUE(container.empty());

  // If we have the growable trait, then attempt to add
  if (ep::Growable<TypeParam::HoldsInt>::value)
  {
    testTraits::addItem(container, 10);
    EXPECT_EQ(1, container.size());
    EXPECT_FALSE(container.empty());
  }
}


REGISTER_TYPED_TEST_CASE_P(Traits_HasSizeTest, SizeAndEmpty);


#endif  // _EP_TEST_TRAITS_H
