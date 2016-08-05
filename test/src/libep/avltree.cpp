#include "eptest.h"
#include "traits/hassize.h"
#include "traits/isgrowable.h"
#include "traits/isshrinkable.h"
#include "ep/cpp/avltree.h"
#include "maptests.h"

using ep::AVLTree;
using ep::SharedString;

// Traits

using TestAVLTree = AVLTree<ep::SharedString, int>;
using TestAVLTreeCIP = AVLTree<ep::SharedString, const int*>;

static_assert(std::is_same<ep::IndexType<TestAVLTree>, ep::SharedString>::value == true, "ep::IndexType failed!");

static_assert(std::is_same<ep::ElementType<TestAVLTree>, int>::value == true, "ep::ElementType failed!");
static_assert(std::is_same<ep::ElementType<TestAVLTreeCIP>, const int*>::value == true, "ep::ElementType failed!");

static_assert(ep::HasFront<TestAVLTree>::value == false, "ep::HasFront failed!");
static_assert(ep::HasBack<TestAVLTree>::value == false, "ep::HasBack failed!");
static_assert(ep::RandomAccessible<TestAVLTree>::value == true, "ep::RandomAccessible failed!");

static_assert(ep::HasSize<TestAVLTree>::value == true, "ep::HasSize failed!");
static_assert(ep::IsContainer<TestAVLTree>::value == true, "ep::IsContainer failed!");

static_assert(ep::Growable<TestAVLTree>::value == true, "ep::Growable failed!");
static_assert(ep::Shrinkable<TestAVLTree>::value == true, "ep::Shrinkable failed!");
static_assert(ep::IsMutable< TestAVLTree>::value == true, "ep::IsMutable failed!");

static_assert(ep::IsKeyed<TestAVLTree>::value == true, "ep::IsKeyed failed!");


DEFINE_MAP_TEST_CONTAINER_KEYED(ep::AVLTree, ep::SharedString)
using MyTypes = typename ::testing::Types<testTraits::Types>;
INSTANTIATE_TYPED_TEST_CASE_P(AVLTree, Traits_HasSizeTest, MyTypes);
INSTANTIATE_TYPED_TEST_CASE_P(AVLTree, Traits_IsGrowable, MyTypes);
INSTANTIATE_TYPED_TEST_CASE_P(AVLTree, Traits_IsShrinkable, MyTypes);
INSTANTIATE_TYPED_TEST_CASE_P(AVLTree, Traits_IsGrowable_RandomAccessible, MyTypes);
INSTANTIATE_TYPED_TEST_CASE_P(AVLTree, Traits_IsShrinkable_RandomAccessible, MyTypes);
INSTANTIATE_TYPED_TEST_CASE_P(AVLTree, Maps_Coverage, MyTypes);
INSTANTIATE_TYPED_TEST_CASE_P(AVLTree, Maps_InsertAndRemoveTest, MyTypes);
// TODO: Enable this once find() and remove are implemented
//INSTANTIATE_TYPED_TEST_CASE_P(AVLTree, Maps_Iterators, MyTypes);

