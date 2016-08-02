#include "../traits.h"

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
