#include "eptest.h"
#include "traits.h"
#include "traits/hassize.h"
#include "traits/isgrowable.h"
#include "traits/isshrinkable.h"
#include "ep/cpp/hashmap.h"
#include "maptests.h"

using ep::Hash;
using ep::HashMap;
using ep::SharedString;

namespace hashmap_test {

class TestValue
{
public:
  TestValue(int val)
  {
    ++numInstances;
    instanceData = val;
  }

  TestValue(const TestValue &test)
    : instanceData(test.instanceData)
  {
    ++numInstances;
  }

  TestValue(TestValue &&rval)
  {
    ++numInstances;
    instanceData = rval.instanceData;
    rval.instanceData = 0;
  }

  ~TestValue()
  {
    --numInstances;
  }

  TestValue& operator=(const TestValue &test)
  {
    instanceData = test.instanceData;
    return *this;
  }

  static int numInstances;
  int instanceData = 0;
};

int TestValue::numInstances = 0;


template <typename K, typename V>
int FindInstances(const ep::KVP<K, V> *keyTable, size_t tableSize, const ep::HashMap<K, V> &map)
{
  int foundCount = 0;
  auto it = map.begin();

  while (it != map.end())
  {
    for (size_t i = 0; i < tableSize; ++i)
    {
      if ((*it).key == keyTable[i].key && (*it).value.instanceData == keyTable[i].value.instanceData)
      {
        ++foundCount;
        break;
      }
    }
    ++it;
  }

  return foundCount;
}

} // namespace hashmap_test


using TestHashMap = HashMap<ep::SharedString, int>;
using TestHashMapCIP = HashMap<ep::SharedString, const int*>;

static_assert(std::is_same<ep::IndexType<TestHashMap>, ep::SharedString>::value == true, "ep::IndexType failed!");

static_assert(std::is_same<ep::ElementType<TestHashMap>, int>::value == true, "ep::ElementType failed!");
static_assert(std::is_same<ep::ElementType<TestHashMapCIP>, const int*>::value == true, "ep::ElementType failed!");

static_assert(ep::HasFront<TestHashMap>::value == false, "ep::HasFront failed!");
static_assert(ep::HasBack<TestHashMap>::value == false, "ep::HasBack failed!");
static_assert(ep::RandomAccessible<TestHashMap>::value == true, "ep::RandomAccessible failed!");

static_assert(ep::HasSize<TestHashMap>::value == true, "ep::HasSize failed!");
static_assert(ep::IsContainer<TestHashMap>::value == true, "ep::IsContainer failed!");

static_assert(ep::Growable<TestHashMap>::value == true, "ep::Growable failed!");
static_assert(ep::Shrinkable<TestHashMap>::value == true, "ep::Shrinkable failed!");
static_assert(ep::IsMutable< TestHashMap>::value == true, "ep::IsMutable failed!");

static_assert(ep::IsKeyed<TestHashMap>::value == true, "ep::IsKeyed failed!");
// Traits

DEFINE_MAP_TEST_CONTAINER_KEYED(ep::HashMap, ep::SharedString)
using MyTypes = typename ::testing::Types<testTraits::Types>;
INSTANTIATE_TYPED_TEST_CASE_P(HashMap, Traits_HasSizeTest, MyTypes);
INSTANTIATE_TYPED_TEST_CASE_P(HashMap, Traits_IsGrowable, MyTypes);
INSTANTIATE_TYPED_TEST_CASE_P(HashMap, Traits_IsShrinkable, MyTypes);
INSTANTIATE_TYPED_TEST_CASE_P(HashMap, Traits_IsGrowable_RandomAccessible, MyTypes);
INSTANTIATE_TYPED_TEST_CASE_P(HashMap, Traits_IsShrinkable_RandomAccessible, MyTypes);
INSTANTIATE_TYPED_TEST_CASE_P(HashMap, Maps_Coverage, MyTypes);
INSTANTIATE_TYPED_TEST_CASE_P(HashMap, Maps_InsertAndRemoveTest, MyTypes);
INSTANTIATE_TYPED_TEST_CASE_P(HashMap, Maps_Iterators, MyTypes);

// ------------------------------- Hash Tests ----------------------------------------------

TEST(Hash, SharedStringHashTests)
{
  EXPECT_TRUE(Hash<SharedString>::eq("", ""));
  EXPECT_TRUE(Hash<SharedString>::eq("bob", "bob"));
  EXPECT_FALSE(Hash<SharedString>::eq("bob", "bOb"));
  EXPECT_FALSE(Hash<SharedString>::eq("bobbington", "bob"));
  uint32_t hash1 = Hash<SharedString>::hash("bob");
  EXPECT_EQ(hash1, Hash<SharedString>::hash("bob"));
  EXPECT_NE(hash1, Hash<SharedString>::hash("zebra"));

  // TODO: Update this when the hash<T> gets updated
  EXPECT_TRUE(Hash<int>::eq(10, 10));
  EXPECT_FALSE(Hash<int>::eq(2, 10));
  EXPECT_EQ(0, Hash<int>::hash(100));
  EXPECT_EQ(0, Hash<int>::hash(999));

  // TODO: Add other types when it makes sense...
}

TEST(HashMap, Constructors)
{
  using Test = HashMap<SharedString, int>;
  EXPECT_DEATH_FROM_ASSERT(Test map(99), "tableSize must be power-of-2!");

  HashMap<SharedString, int> map2(8);
  EXPECT_EQ(0, map2.size());
  map2.insert("bob", 10);
  EXPECT_EQ(1, map2.size());

  HashMap<SharedString, int> map3 = std::move(map2);
  EXPECT_EQ(1, map3.size());
  EXPECT_EQ(0, map2.size());
}
