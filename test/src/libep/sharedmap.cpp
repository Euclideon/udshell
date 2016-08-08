#include "eptest.h"
#include "traits/hassize.h"
#include "traits/isgrowable.h"
#include "traits/isshrinkable.h"
#include "ep/cpp/avltree.h"
#include "ep/cpp/map.h"
#include "maptests.h"

using ep::AVLTree;
using ep::SharedMap;
using ep::SharedString;

using TestMAP = SharedMap<AVLTree<ep::SharedString, int>>;
using TestMAPCIP = SharedMap<AVLTree<ep::SharedString, const int*>>;

static_assert(std::is_same<ep::IndexType<TestMAP>, ep::SharedString>::value == true, "ep::IndexType failed!");

static_assert(std::is_same<ep::ElementType<TestMAP>, int>::value == true, "ep::ElementType failed!");
static_assert(std::is_same<ep::ElementType<TestMAPCIP>, const int*>::value == true, "ep::ElementType failed!");

static_assert(ep::HasFront<TestMAP>::value == false, "ep::HasFront failed!");
static_assert(ep::HasBack<TestMAP>::value == false, "ep::HasBack failed!");
static_assert(ep::RandomAccessible<TestMAP>::value == true, "ep::RandomAccessible failed!");

static_assert(ep::HasSize<TestMAP>::value == true, "ep::HasSize failed!");
static_assert(ep::IsContainer<TestMAP>::value == true, "ep::IsContainer failed!");

static_assert(ep::Growable<TestMAP>::value == true, "ep::Growable failed!");
static_assert(ep::Shrinkable<TestMAP>::value == true, "ep::Shrinkable failed!");
static_assert(ep::IsMutable< TestMAP>::value == true, "ep::IsMutable failed!");

static_assert(ep::IsKeyed<TestMAP>::value == true, "ep::IsKeyed failed!");

// Traits

DEFINE_MAP_TEST_CONTAINER_TREE(SharedMap, AVLTree, SharedString)
using MyTypes = typename ::testing::Types<testTraits::Types>;
INSTANTIATE_TYPED_TEST_CASE_P(SharedMap, Traits_HasSizeTest, MyTypes);
INSTANTIATE_TYPED_TEST_CASE_P(SharedMap, Traits_IsGrowable, MyTypes);
INSTANTIATE_TYPED_TEST_CASE_P(SharedMap, Traits_IsShrinkable, MyTypes);
INSTANTIATE_TYPED_TEST_CASE_P(SharedMap, Traits_IsGrowable_RandomAccessible, MyTypes);
INSTANTIATE_TYPED_TEST_CASE_P(SharedMap, Traits_IsShrinkable_RandomAccessible, MyTypes);
INSTANTIATE_TYPED_TEST_CASE_P(SharedMap, Maps_Coverage, MyTypes);
INSTANTIATE_TYPED_TEST_CASE_P(SharedMap, Maps_InsertAndRemoveTest, MyTypes);
// TODO: Enable this once find() and remove are implemented
//INSTANTIATE_TYPED_TEST_CASE_P(SharedMap, Maps_Iterators, MyTypes);

