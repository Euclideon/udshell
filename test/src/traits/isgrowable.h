#pragma once
#if !defined(_EP_IS_GROWABLE_H)
#define _EP_IS_GROWABLE_H

#include "../traits.h"
#include "ep/cpp/string.h"
#include "ep/cpp/keyvaluepair.h"

template <typename T>
class Traits_IsGrowable : public ::testing::Test { };
TYPED_TEST_CASE_P(Traits_IsGrowable);

TYPED_TEST_P(Traits_IsGrowable, Growable)
{
  EXPECT_TRUE(ep::Growable<typename TypeParam::DefaultType>::value);
}

template <typename T>
class Traits_IsGrowable_HasFront : public ::testing::Test { };
TYPED_TEST_CASE_P(Traits_IsGrowable_HasFront);

TYPED_TEST_P(Traits_IsGrowable_HasFront, PushFront)
{
  EXPECT_TRUE(ep::Growable<typename TypeParam::DefaultType>::value && ep::HasFront<typename TypeParam::DefaultType>::value);

  // Default container is empty
  typename TypeParam::DefaultType container;
  EXPECT_TRUE((std::is_same<ep::ElementType<decltype(container)>, int>::value));

  container.push_front(1);
  EXPECT_EQ(1, container.front());
}

template <typename T>
class Traits_IsGrowable_HasBack : public ::testing::Test { };
TYPED_TEST_CASE_P(Traits_IsGrowable_HasBack);

TYPED_TEST_P(Traits_IsGrowable_HasBack, PushBack)
{
  EXPECT_TRUE(ep::Growable<typename TypeParam::DefaultType>::value && ep::HasBack<typename TypeParam::DefaultType>::value);

  // Default container is empty
  typename TypeParam::DefaultType container;
  EXPECT_TRUE((std::is_same<ep::ElementType<decltype(container)>, int>::value));

  container.push_back(1);
  EXPECT_EQ(1, container.back());
}

template <typename T>
class Traits_IsGrowable_RandomAccessible : public ::testing::Test { };
TYPED_TEST_CASE_P(Traits_IsGrowable_RandomAccessible);

TYPED_TEST_P(Traits_IsGrowable_RandomAccessible, Insert)
{
  EXPECT_TRUE(ep::Growable<typename TypeParam::DefaultType>::value && ep::RandomAccessible<typename TypeParam::DefaultType>::value);

  // Default container is empty
  typename TypeParam::DefaultType container;
  EXPECT_TRUE((std::is_same<ep::ElementType<decltype(container)>, int>::value));

  container.insert(testTraits::value<typename TypeParam::DefaultType>(1));
  EXPECT_EQ(1, container.at(testTraits::key<typename TypeParam::DefaultType>(1)));
}

REGISTER_TYPED_TEST_CASE_P(Traits_IsGrowable, Growable);
REGISTER_TYPED_TEST_CASE_P(Traits_IsGrowable_HasFront, PushFront);
REGISTER_TYPED_TEST_CASE_P(Traits_IsGrowable_HasBack, PushBack);
REGISTER_TYPED_TEST_CASE_P(Traits_IsGrowable_RandomAccessible, Insert);

#endif  // _EP_IS_GROWABLE_H
