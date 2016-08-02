#pragma once
#if !defined(_EP_IS_SHRINKABLE_H)
#define _EP_IS_SHRINKABLE_H

#include "../traits.h"
#include "ep/cpp/string.h"
#include "ep/cpp/keyvaluepair.h"

template <typename T>
class Traits_IsShrinkable : public ::testing::Test { };
TYPED_TEST_CASE_P(Traits_IsShrinkable);

TYPED_TEST_P(Traits_IsShrinkable, Shrinkable)
{
  EXPECT_TRUE(ep::Shrinkable<TypeParam::HoldsInt>::value);
}

template <typename T>
class Traits_IsShrinkable_HasFront : public ::testing::Test { };
TYPED_TEST_CASE_P(Traits_IsShrinkable_HasFront);

TYPED_TEST_P(Traits_IsShrinkable_HasFront, PopFront)
{
  EXPECT_TRUE(ep::Shrinkable<TypeParam::HoldsInt>::value && ep::HasFront<TypeParam::HoldsInt>::value);

  // Default container is empty
  TypeParam::HoldsInt container = { 0, 1 };
  EXPECT_TRUE((std::is_same<ep::ElementType<decltype(container)>, int>::value));

  EXPECT_EQ(0, container.front());

  container.pop_front(1);
  EXPECT_EQ(1, container.front());

}

template <typename T>
class Traits_IsShrinkable_HasBack : public ::testing::Test { };
TYPED_TEST_CASE_P(Traits_IsShrinkable_HasBack);

TYPED_TEST_P(Traits_IsShrinkable_HasBack, PopBack)
{
  EXPECT_TRUE(ep::Growable<TypeParam::HoldsInt>::value && ep::HasBack<TypeParam::HoldsInt>::value);

  // Default container is empty
  TypeParam::HoldsInt container = { 0, 1 };
  EXPECT_TRUE((std::is_same<ep::ElementType<decltype(container)>, int>::value));

  EXPECT_EQ(1, container.back());

  container.pop_back(1);
  EXPECT_EQ(0, container.back());
}

template <typename T>
class Traits_IsShrinkable_RandomAccessible : public ::testing::Test { };
TYPED_TEST_CASE_P(Traits_IsShrinkable_RandomAccessible);

TYPED_TEST_P(Traits_IsShrinkable_RandomAccessible, Remove)
{
  EXPECT_TRUE(ep::Growable<TypeParam::HoldsInt>::value && ep::RandomAccessible<TypeParam::HoldsInt>::value);

  // Default container is empty
  TypeParam::HoldsInt container = { testTraits::value<TypeParam::HoldsInt>(0), testTraits::value<TypeParam::HoldsInt>(1) };
  EXPECT_TRUE((std::is_same<ep::ElementType<decltype(container)>, int>::value));

  EXPECT_EQ(0, container.at(testTraits::key<TypeParam::HoldsInt>(0)));

  container.remove(testTraits::key<TypeParam::HoldsInt>(0));
  EXPECT_EQ(1, container.at(testTraits::key<TypeParam::HoldsInt>(1)));
}

REGISTER_TYPED_TEST_CASE_P(Traits_IsShrinkable, Shrinkable);
REGISTER_TYPED_TEST_CASE_P(Traits_IsShrinkable_HasFront, PopFront);
REGISTER_TYPED_TEST_CASE_P(Traits_IsShrinkable_HasBack, PopBack);
REGISTER_TYPED_TEST_CASE_P(Traits_IsShrinkable_RandomAccessible, Remove);

#endif  // _EP_IS_SHRINKABLE_H
