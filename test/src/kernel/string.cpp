#include "gtest/gtest.h"
#include "ep/epplatform.h"
// TODO: fill out these tests

TEST(EPStringTest, EmptyString) {
  epString nullStr;
  EXPECT_TRUE(nullStr.empty());
  EXPECT_FALSE(nullStr);  // operator bool
  EXPECT_TRUE(!nullStr);  // operator bool
  EXPECT_EQ(0, nullStr.length);
  EXPECT_EQ(nullptr, nullStr.ptr);
}

TEST(EPStringTest, NotEmptyString) {
  epString nullStr("blah");
  EXPECT_FALSE(nullStr.empty());
  EXPECT_FALSE(!nullStr);   // operator bool
  EXPECT_TRUE(nullStr);     // operator bool
  EXPECT_EQ(4, nullStr.length);
  EXPECT_TRUE(nullStr.ptr != nullptr);
}
