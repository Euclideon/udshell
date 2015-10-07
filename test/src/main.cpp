#include "gtest/gtest.h"
#include "ep/epplatform.h"

namespace ep_internal
{
  extern bool gUnitTesting;
}

int main(int argc, char **argv)
{
  ep_internal::gUnitTesting = true;
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
