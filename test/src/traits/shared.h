#pragma once
#if !defined(_EP_SHARED_H)
#define _EP_SHARED_H

#include "../traits.h"

template <typename T>
class Traits_Shared : public ::testing::Test { };
TYPED_TEST_CASE_P(Traits_Shared);


TYPED_TEST_P(Traits_Shared, CreateNull)
{
  // default construction is null
  typename TypeParam::DefaultType spDefault;
  ASSERT_FALSE(spDefault);
  EXPECT_TRUE(!spDefault);
  EXPECT_EQ(0, spDefault.use_count());

  // explicit null construction
  typename TypeParam::DefaultType spNull = nullptr;
  ASSERT_FALSE(spNull);
  EXPECT_TRUE(!spNull);
  EXPECT_EQ(0, spNull.use_count());
}

TYPED_TEST_P(Traits_Shared, IncAndDec)
{
  auto obj = TypeParam::create();

  ASSERT_TRUE(obj);
  EXPECT_EQ(1, obj.use_count());

  EXPECT_EQ(2, obj.incRef());
  ASSERT_TRUE(obj);
  EXPECT_EQ(2, obj.use_count());

  EXPECT_EQ(1, obj.decRef());
  ASSERT_TRUE(obj);
  EXPECT_EQ(1, obj.use_count());

  EXPECT_EQ(0, obj.decRef());
  ASSERT_FALSE(obj);
  EXPECT_EQ(0, obj.use_count());
}

TYPED_TEST_P(Traits_Shared, CopyAssign)
{
  auto obj = TypeParam::create();

  typename TypeParam::DefaultType copy = obj;
  EXPECT_TRUE(copy != nullptr);
  EXPECT_TRUE(copy == obj);
  EXPECT_EQ(2, copy.use_count());
  EXPECT_EQ(2, obj.use_count());

  {
    typename TypeParam::DefaultType copy2 = copy;
    EXPECT_TRUE(copy2 != nullptr);
    EXPECT_TRUE(copy2 == copy);
    EXPECT_TRUE(copy2 == obj);
    EXPECT_EQ(3, copy2.use_count());
    EXPECT_EQ(3, copy.use_count());
    EXPECT_EQ(3, obj.use_count());
  }

  EXPECT_EQ(2, copy.use_count());
  EXPECT_EQ(2, obj.use_count());

  copy = nullptr;
  EXPECT_TRUE(copy == nullptr);
  EXPECT_EQ(1, obj.use_count());
}

TYPED_TEST_P(Traits_Shared, MoveAssign)
{
  auto obj = TypeParam::create();

  typename TypeParam::DefaultType copy = std::move(obj);

  EXPECT_TRUE(copy != nullptr);
  EXPECT_TRUE(copy != obj);
  EXPECT_TRUE(obj == nullptr);
  EXPECT_EQ(1, copy.use_count());
  EXPECT_EQ(0, obj.use_count());

  {
    typename TypeParam::DefaultType copy2 = std::move(copy);

    EXPECT_TRUE(copy2 != nullptr);
    EXPECT_TRUE(copy2 != copy);
    EXPECT_TRUE(copy == nullptr);
    EXPECT_EQ(1, copy2.use_count());
    EXPECT_EQ(0, copy.use_count());
  }

  EXPECT_EQ(0, copy.use_count());
  EXPECT_EQ(0, obj.use_count());
}

TYPED_TEST_P(Traits_Shared, ConstPromotion)
{
  auto obj = TypeParam::create();

  // TODO: this needs to work for SharedArray, and SharedString!!
//  typename TypeParam::ConstType copy = obj;
//
//  EXPECT_TRUE(copy != nullptr);
//  EXPECT_TRUE(copy == obj);
//  EXPECT_EQ(2, copy.use_count());
//
//  const typename TypeParam::Type copy2 = obj;
//
//  EXPECT_TRUE(copy2 != nullptr);
//  EXPECT_TRUE(copy2 == obj);
//  EXPECT_EQ(3, copy2.use_count());
//
//  const typename TypeParam::ConstType copy3 = copy;
//
//  EXPECT_TRUE(copy3 != nullptr);
//  EXPECT_TRUE(copy3 == copy);
//  EXPECT_EQ(4, copy3.use_count());
//
//  const typename TypeParam::ConstType copy4 = copy2;
//
//  EXPECT_TRUE(copy4 != nullptr);
//  EXPECT_TRUE(copy4 == copy2);
//  EXPECT_EQ(5, copy4.use_count());
}

TYPED_TEST_P(Traits_Shared, Unique)
{
  auto obj = TypeParam::create();

  EXPECT_TRUE(obj.unique());

  {
    typename TypeParam::DefaultType copy = obj;
    EXPECT_FALSE(copy.unique());
  }

  EXPECT_TRUE(obj.unique());
}


REGISTER_TYPED_TEST_CASE_P(Traits_Shared, CreateNull, IncAndDec, CopyAssign, MoveAssign, ConstPromotion, Unique);

#endif // _EP_SHARED_H
