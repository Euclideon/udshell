#include "eptest.h"

namespace ep {
namespace internal {

extern bool gUnitTesting;

// HACK !!! (GCC and Clang)
// For the implementation fo epInternalInit defined in globalinitialisers to override
// the weak version in epplatform.cpp at least one symbol from that file must
// be referenced externally.
void *getStaticImplRegistry();

} // namespace internal
} // namespace ep

int main(int argc, char **argv)
{
  ep::internal::getStaticImplRegistry();
  ep::internal::gUnitTesting = true;
  testing::InitGoogleTest(&argc, argv);
  int result = RUN_ALL_TESTS();
  return result;
}
