#pragma once
#if !defined(_EP_TEST_H)
#define _EP_TEST_H
#include "ep/cpp/platform.h"
#include "gtest/gtest.h"

#if EP_DEBUG
# define EXPECT_DEATH_FROM_ASSERT EXPECT_DEATH
# define EXPECT_ASSERT_THROW(condition) EXPECT_DEATH(condition, "")
#else
# define EXPECT_DEATH_FROM_ASSERT(...)
# define EXPECT_ASSERT_THROW(condition) EXPECT_THROW(condition, ep::EPException)
#endif

#endif // _EP_TEST_H

