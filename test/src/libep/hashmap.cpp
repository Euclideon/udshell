#include "eptest.h"
#include "ep/cpp/hashmap.h"

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

TEST(HashMap, InsertAndRemove)
{
  HashMap<SharedString, int> map;
  EXPECT_TRUE(map.empty());

  // Standard inserts
  EXPECT_NO_THROW(map.insert("bob", 1));
  EXPECT_EQ(1, map.size());
  EXPECT_NO_THROW(map.insert(ep::KVP<SharedString, int>("sarah", 2)));
  EXPECT_EQ(2, map.size());

  // Check our move assigns aren't trashing
  using namespace hashmap_test;
  HashMap<SharedString, TestValue> testMap;
  EXPECT_NO_THROW(testMap.insert("bill", TestValue(10)));
  EXPECT_EQ(1, testMap.size());
  EXPECT_EQ(1, TestValue::numInstances);

  {
    TestValue localInstance(99);
    EXPECT_NO_THROW(testMap.insert("nigel", localInstance));
    EXPECT_EQ(3, TestValue::numInstances);
    EXPECT_EQ(99, localInstance.instanceData);
    EXPECT_EQ(99, testMap["nigel"].instanceData);
  }
  EXPECT_EQ(2, TestValue::numInstances);
  EXPECT_EQ(2, testMap.size());

  // Lazy insert
  // Lazy insert has been disabled until ep's delegate support lambdas
  // TODO: Change this back to lazy insert after its been fixed.
  //testMap.tryInsert("gordon", []() -> TestValue { return TestValue(101); });
  testMap.insert("gordon", TestValue(101));
  EXPECT_EQ(3, TestValue::numInstances);
  EXPECT_EQ(3, testMap.size());
  EXPECT_EQ(101, testMap["gordon"].instanceData);

  // Accessing invalid key
  EXPECT_ASSERT_THROW(testMap["mary"]);

  // Gets
  EXPECT_EQ(nullptr, testMap.get("optimus prime"));
  EXPECT_EQ(101, testMap.get("gordon")->instanceData);

  // Remove a key that doesn't exist
  testMap.remove("optimus prime");
  EXPECT_EQ(3, testMap.size());

  // Remove a key that does
  testMap.remove("gordon");
  EXPECT_EQ(2, testMap.size());

  // Clear
  testMap.clear();
  EXPECT_EQ(0, testMap.size());
  EXPECT_TRUE(testMap.empty());

  // Insert and then insert replace
  EXPECT_NO_THROW(testMap.insert("bob", TestValue(10)));
  EXPECT_EQ(10, testMap["bob"].instanceData);
  EXPECT_EQ(1, TestValue::numInstances);
  EXPECT_EQ(1, testMap.size());
  EXPECT_THROW(testMap.insert("bob", TestValue(11)), ep::EPException);
  EXPECT_EQ(10, testMap["bob"].instanceData);
  EXPECT_EQ(1, TestValue::numInstances);
  EXPECT_EQ(1, testMap.size());
  testMap.replace("bob", TestValue(12));
  EXPECT_EQ(12, testMap["bob"].instanceData);
  EXPECT_EQ(1, TestValue::numInstances);
  EXPECT_EQ(1, testMap.size());

  // Lazy insert has been disabled until ep's delegate support lambdas
  // TODO: Renable this after its been fixed.
#if 0
  // Insert lazy when it already exists - shouldn't replace
  testMap.tryInsert("bob", []() -> TestValue { return TestValue(101); });
  EXPECT_EQ(12, testMap.get("bob")->instanceData);
  EXPECT_EQ(1, TestValue::numInstances);
  EXPECT_EQ(1, testMap.size());
#endif
}

TEST(HashMap, Iterators)
{
  using namespace hashmap_test;
  HashMap<SharedString, TestValue> smallMap(8, 4);
  const int TABLE_SIZE = 8;
  ep::KVP<SharedString, TestValue> keyTable[TABLE_SIZE] =
    {
      { "uno", 0 },
      { "dos", 1 },
      { "tre", 2 },
      { "cuatro", 3 },
      { "cinco", 4 },
      { "seis", 5 },
      { "siete", 6 },
      { "ocho", 7 },
    };
  for (int i = 0; i < TABLE_SIZE; ++i)
    smallMap.insert(keyTable[i]);

  EXPECT_EQ(TABLE_SIZE, smallMap.size());

  auto it = smallMap.begin();
  EXPECT_TRUE(it != smallMap.end());

  // locate all the keys in the table using an iterator
  EXPECT_EQ(TABLE_SIZE, (FindInstances<SharedString, TestValue>(keyTable, TABLE_SIZE, smallMap)));

  // Locate a key
  it = smallMap.find("cinco");
  EXPECT_EQ(4, it->instanceData);

  // Locate the items on either side of cinco in the table
  auto prev = smallMap.begin();
  auto next = prev;
  while (next != smallMap.end())
  {
    ++next;
    if (next == it)
      break;
    prev = next;
  }
  ++next;

  // 'it' should point to cinco
  // 'prev' should point to the previous item in the table
  // 'next' should point to the next item in the table
  EXPECT_TRUE(it != prev);
  EXPECT_TRUE(it != next);
  EXPECT_TRUE(next != prev);
  int valPrev = prev->instanceData;
  int valNext = next->instanceData;

  // Now remove the key and verify the iterators are still valid
  it = smallMap.erase(it);
  EXPECT_TRUE(it == next);
  EXPECT_EQ(valPrev, prev->instanceData);
  EXPECT_EQ(valNext, next->instanceData);

  // Verify that incrementing the returned iterator is still valid
  auto tempItr = next;
  EXPECT_TRUE(++it == ++tempItr);

  // Now increment prev, and it should point to next (and skip over the removed value)
  ++prev;
  EXPECT_EQ(prev, next);

  EXPECT_EQ(TABLE_SIZE-1, smallMap.size());

  // Attempt to find a key that doesn't exist
  EXPECT_EQ(smallMap.end(), smallMap.find("bob"));

  // Attempt to erase an invalid iterator
  EXPECT_ASSERT_THROW(smallMap.erase(smallMap.end()));

  // Insert past the proposed size and verify all our members exist
  HashMap<SharedString, int> tinyMap(2);
  const int NUM_ENTRIES = 10;
  char keys[NUM_ENTRIES];
  int values[NUM_ENTRIES];
  int expSum = 0;
  for (int i = 0; i < NUM_ENTRIES; ++i)
  {
    keys[i] = 'a' + (char)i;
    values[i] = i;
    expSum += i;
    tinyMap.insert(ep::String(&keys[i], 1), values[i]);
  }

  EXPECT_EQ(NUM_ENTRIES, tinyMap.size());

  // Use a for each to iterate and verify the keys and values
  int sum = 0;
  for (auto a : tinyMap)
  {
    sum += a.value;
    EXPECT_TRUE(a.key.eq(ep::String(&keys[a.value], 1)));
  }
  EXPECT_EQ(expSum, sum);

  // Remove the first element
  auto firstIter = tinyMap.begin();
  auto nextIter = firstIter;
  ++nextIter;
  EXPECT_EQ(nextIter, tinyMap.erase(firstIter));
  EXPECT_EQ(NUM_ENTRIES-1, tinyMap.size());

  // Remove the last element
  firstIter = tinyMap.begin();
  nextIter = firstIter;
  while(++nextIter != tinyMap.end())
    firstIter = nextIter;

  EXPECT_EQ(nextIter, tinyMap.end());
  EXPECT_EQ(nextIter, tinyMap.erase(firstIter));
  EXPECT_EQ(NUM_ENTRIES-2, tinyMap.size());

  // Iterate and remove each remaining element
  nextIter = tinyMap.begin();
  sum = 0;
  while (nextIter != tinyMap.end())
  {
    nextIter = tinyMap.erase(nextIter);
    ++sum;
  }

  EXPECT_TRUE(tinyMap.empty());
  EXPECT_EQ(NUM_ENTRIES-2, sum);
}
