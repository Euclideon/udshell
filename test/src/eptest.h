#pragma once
#if !defined(_EP_TEST_H)
#define _EP_TEST_H
#include "ep/c/platform.h"
#include "gtest/gtest.h"

#if EP_DEBUG
# define EXPECT_DEATH_FROM_ASSERT EXPECT_DEATH
#else
# define EXPECT_DEATH_FROM_ASSERT(...)
#endif

#endif // _EP_TEST_H

