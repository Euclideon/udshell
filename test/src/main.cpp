#include "gtest/gtest.h"
#include "ep/cpp/platform.h"

namespace ep {
namespace internal {

extern bool gUnitTesting;

} // namespace internal
} // namespace ep

int main(int argc, char **argv)
{
  internal::gUnitTesting = true;
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
