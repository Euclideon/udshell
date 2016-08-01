#include "eptest.h"
#include "traits.h"
#include "ep/cpp/traits.h"
#include "ep/cpp/slice.h"


// !! TEMP EXAMPLE OF USAGE

DEFINE_TEST_CONTAINER(ep::Slice)
using MyTypes = typename ::testing::Types<testTraits::Types>;
INSTANTIATE_TYPED_TEST_CASE_P(Slice, Traits_HasSizeTest, MyTypes);
