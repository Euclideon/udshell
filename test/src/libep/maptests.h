#pragma once
#if !defined(_EP_MAP_TEST_H)
#define _EP_MAP_TEST_H
#include "../traits.h"


namespace map_test {

  template <typename T>
  class TestValueType
  {
  public:
    TestValueType(int val)
    {
      ++numInstances();
      instanceData = val;
    }

    TestValueType(const TestValueType &test)
      : instanceData(test.instanceData)
    {
      ++numInstances();
    }

    TestValueType(TestValueType &&rval)
    {
      ++numInstances();
      instanceData = rval.instanceData;
      rval.instanceData = 0;
    }

    ~TestValueType()
    {
      --numInstances();
    }

    TestValueType& operator=(const TestValueType &test)
    {
      instanceData = test.instanceData;
      return *this;
    }

    static int &numInstances()
    {
      static int numInstances = 0;
      return numInstances;
    }

    int instanceData = 0;
    T *pUnique = nullptr;
  };


  template <typename T>
  int FindInstances(const typename T::KeyValuePair *keyTable, size_t tableSize, const T &map)
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

  template <typename T>
  class IsHashMap
  {
    template <typename U> static auto Impl(int) -> decltype(T(size_t(0), size_t(0)), std::true_type());
    template <typename> static std::false_type Impl(...);
  public:

    using type = decltype(Impl<T>(0));
    static constexpr bool value = type::value;
  };

  template <typename T>
  inline T ConstructMap(size_t t, size_t b)
  {
    if (IsHashMap<T>::value)
    {
      return T(t, b);
    }

    return T();
  }

} // namespace map_test


#define DEFINE_MAP_TEST_CONTAINER_KEYED(Container, KeyType)                                 \
  namespace testTraits {                                                                    \
  struct Types {                                                                            \
    using DefaultType = Container<KeyType, int>;                                            \
    using HoldsInt = Container<KeyType, int>;                                               \
    using HoldsFloat = Container<KeyType, float>;                                           \
    using HoldsDouble = Container<KeyType, double>;                                         \
    using HoldsBool = Container<KeyType, bool>;                                             \
    using HoldsChar = Container<KeyType, char>;                                             \
    using HoldsString = Container<KeyType, ep::SharedString>;                               \
    using HoldsTestValue = Container<KeyType, map_test::TestValueType<HoldsInt>>;           \
    static DefaultType create()                                                             \
    {                                                                                       \
      return DefaultType({ { KeyType(), 10 } });                                            \
    }                                                                                       \
  };                                                                                        \
  }

#define DEFINE_MAP_TEST_CONTAINER_TREE(Container, TreeType, KeyType)                        \
  namespace testTraits {                                                                    \
  struct Types {                                                                            \
    using DefaultType = Container<TreeType<KeyType, int>>;                                  \
    using HoldsInt = Container<TreeType<KeyType, int>>;                                     \
    using HoldsFloat = Container<TreeType<KeyType, float>>;                                 \
    using HoldsDouble = Container<TreeType<KeyType, double>>;                               \
    using HoldsBool = Container<TreeType<KeyType, bool>>;                                   \
    using HoldsChar = Container<TreeType<KeyType, char>>;                                   \
    using HoldsString = Container<TreeType<KeyType, ep::SharedString>>;                     \
    using HoldsTestValue = Container<TreeType<KeyType, map_test::TestValueType<HoldsInt>>>; \
    static DefaultType create()                                                             \
    {                                                                                       \
      return DefaultType({ { KeyType(), 10 } });                                            \
    }                                                                                       \
  };                                                                                        \
  }

template <typename T>
class Maps_Coverage : public ::testing::Test { };
TYPED_TEST_CASE_P(Maps_Coverage);

TYPED_TEST_P(Maps_Coverage, Coverage)
{
  typename TypeParam::HoldsInt map;
  const typename TypeParam::HoldsInt &constMap = map;
  EXPECT_TRUE(map.empty());

  // -------- Inserts --------
  // lvalue, lvalue
  ep::SharedString bob = "bob";
  int var = 1;
  EXPECT_NO_THROW(map.insert(bob, var));
  EXPECT_EQ(1, map.size());

  EXPECT_THROW(map.insert(bob, var), ep::EPException);
  EXPECT_EQ(1, map.size());

  // rvalue, lvalue
  EXPECT_NO_THROW(map.insert(ep::SharedString("phil"), var));
  EXPECT_EQ(2, map.size());

  EXPECT_THROW(map.insert(ep::SharedString("phil"), var), ep::EPException);
  EXPECT_EQ(2, map.size());

  // lvalue, rvalue
  ep::SharedString rob = "rob";
  EXPECT_NO_THROW(map.insert(rob, int(3)));
  EXPECT_EQ(3, map.size());

  EXPECT_THROW(map.insert(rob, int(3)), ep::EPException);
  EXPECT_EQ(3, map.size());

  // rvalue, rvalue
  EXPECT_NO_THROW(map.insert(ep::SharedString("mick"), int(4)));
  EXPECT_EQ(4, map.size());

  EXPECT_THROW(map.insert(ep::SharedString("mick"), int(4)), ep::EPException);
  EXPECT_EQ(4, map.size());

  // lvalue KVP
  ep::KVP<typename TypeParam::HoldsInt::KeyType, int> kvp("sarah", 5);
  EXPECT_NO_THROW(map.insert(kvp));
  EXPECT_EQ(5, map.size());

  EXPECT_THROW(map.insert(kvp), ep::EPException);
  EXPECT_EQ(5, map.size());

  // rvalue KVP
  EXPECT_NO_THROW(map.insert(ep::KVP<typename TypeParam::HoldsInt::KeyType, int>("joan", 6)));
  EXPECT_EQ(6, map.size());

  EXPECT_THROW(map.insert(ep::KVP<typename TypeParam::HoldsInt::KeyType, int>("joan", 6)), ep::EPException);
  EXPECT_EQ(6, map.size());

  // -------- Accessors --------
  // get const
  EXPECT_EQ(var, *constMap.get(bob));
  EXPECT_EQ(nullptr, constMap.get("frank"));

  // get
  EXPECT_EQ(var, *map.get("phil"));
  EXPECT_EQ(nullptr, map.get("frank"));

  // const operator[]
  EXPECT_NO_THROW(constMap[rob]);
  EXPECT_ASSERT_THROW(constMap["frank"]);
  EXPECT_EQ(3, constMap[rob]);

  // operator[]
  EXPECT_NO_THROW(map["mick"]);
  EXPECT_ASSERT_THROW(map["frank"]);
  EXPECT_EQ(4, map["mick"]);

  // const at()
  EXPECT_NO_THROW(constMap.at("sarah"));
  EXPECT_ASSERT_THROW(constMap.at("frank"));
  EXPECT_EQ(5, constMap.at("sarah"));

  // at()
  EXPECT_NO_THROW(map["joan"]);
  EXPECT_EQ(6, map["joan"]);

  // -------- tryInsert --------
  // lvalue, lvalue
  int tryVar = -1;
  ep::SharedString tryBob = "tryBob";
  EXPECT_EQ(var, map.tryInsert(bob, tryVar));
  EXPECT_EQ(6, map.size());

  EXPECT_EQ(tryVar, map.tryInsert(tryBob, tryVar));
  EXPECT_EQ(7, map.size());

  // rvalue, lvalue
  EXPECT_EQ(var, map.tryInsert(ep::SharedString("phil"), tryVar));
  EXPECT_EQ(7, map.size());

  EXPECT_EQ(tryVar, map.tryInsert(ep::SharedString("tryPhil"), tryVar));
  EXPECT_EQ(8, map.size());

  // lvalue, rvalue
  ep::SharedString tryRob = "tryRob";
  EXPECT_EQ(3, map.tryInsert(rob, int(-3)));
  EXPECT_EQ(8, map.size());

  EXPECT_EQ(-3, map.tryInsert(tryRob, int(-3)));
  EXPECT_EQ(9, map.size());

  // rvalue, rvalue
  EXPECT_EQ(4, map.tryInsert(ep::SharedString("mick"), int(-4)));
  EXPECT_EQ(9, map.size());

  EXPECT_EQ(-4, map.tryInsert(ep::SharedString("tryMick"), int(-4)));
  EXPECT_EQ(10, map.size());

  // try lvalue KVP
  ep::KVP<typename TypeParam::HoldsInt::KeyType, int> tryKvpFail("sarah", -5);
  EXPECT_EQ(5, map.tryInsert(tryKvpFail));
  EXPECT_EQ(10, map.size());

  ep::KVP<typename TypeParam::HoldsInt::KeyType, int> tryKvpSuccess("trySarah", -5);
  EXPECT_EQ(-5, map.tryInsert(tryKvpSuccess));
  EXPECT_EQ(11, map.size());

  // try rvalue KVP
  EXPECT_EQ(6, map.tryInsert(ep::KVP<typename TypeParam::HoldsInt::KeyType, int>("joan", -6)));
  EXPECT_EQ(11, map.size());

  EXPECT_EQ(-6, map.tryInsert(ep::KVP<typename TypeParam::HoldsInt::KeyType, int>("tryJoan", -6)));
  EXPECT_EQ(12, map.size());

  // try lvalue K lazy
  ep::SharedString jim = "jim";
  EXPECT_EQ(-20, map.tryInsert(jim, []() { return -20; }));
  EXPECT_EQ(13, map.size());

  EXPECT_EQ(-20, map.tryInsert(jim, []() { return 0; }));
  EXPECT_EQ(13, map.size());

  // try rvalue K lazy
  EXPECT_EQ(-30, map.tryInsert(ep::SharedString("spock"), []() { return -30; }));
  EXPECT_EQ(14, map.size());

  EXPECT_EQ(-30, map.tryInsert(ep::SharedString("spock"), []() { return 0; }));
  EXPECT_EQ(14, map.size());

  // -------- replace --------
  // lvalue, lvalue
  int replaceVar = 10;
  ep::SharedString replaceBob = "replaceBob";
  EXPECT_EQ(replaceVar, map.replace(bob, replaceVar));
  EXPECT_EQ(14, map.size());

  EXPECT_EQ(replaceVar, map.replace(replaceBob, replaceVar));
  EXPECT_EQ(15, map.size());

  // rvalue, lvalue
  EXPECT_EQ(replaceVar, map.replace(ep::SharedString("phil"), replaceVar));
  EXPECT_EQ(15, map.size());

  EXPECT_EQ(replaceVar, map.replace(ep::SharedString("replacePhil"), replaceVar));
  EXPECT_EQ(16, map.size());

  // lvalue, rvalue
  ep::SharedString replaceRob = "replaceRob";
  EXPECT_EQ(11, map.replace(rob, int(11)));
  EXPECT_EQ(16, map.size());

  EXPECT_EQ(11, map.replace(replaceRob, int(11)));
  EXPECT_EQ(17, map.size());

  // rvalue, rvalue
  EXPECT_EQ(12, map.replace(ep::SharedString("mick"), int(12)));
  EXPECT_EQ(17, map.size());

  EXPECT_EQ(12, map.replace(ep::SharedString("replaceMick"), int(12)));
  EXPECT_EQ(18, map.size());

  // lvalue KVP
  ep::KVP<typename TypeParam::HoldsInt::KeyType, int> kvpReplaceExists("sarah", 13);
  ep::KVP<typename TypeParam::HoldsInt::KeyType, int> kvpReplaceNew("replaceSarah", 13);
  EXPECT_EQ(13, map.replace(kvpReplaceExists));
  EXPECT_EQ(18, map.size());

  EXPECT_EQ(13, map.replace(kvpReplaceNew));
  EXPECT_EQ(19, map.size());

  // rvalue KVP
  EXPECT_EQ(14, map.replace(ep::KVP<typename TypeParam::HoldsInt::KeyType, int>("joan", 14)));
  EXPECT_EQ(19, map.size());

  EXPECT_EQ(14, map.replace(ep::KVP<typename TypeParam::HoldsInt::KeyType, int>("replaceJoan", 14)));
  EXPECT_EQ(20, map.size());

  // -------- remove --------
  map.remove("joan");
  EXPECT_EQ(19, map.size());

  map.remove("frank");
  EXPECT_EQ(19, map.size());

  // -------- exists --------
  EXPECT_TRUE(map.exists(bob));
  EXPECT_FALSE(map.exists("frank"));
}

template <typename T>
class Maps_InsertAndRemoveTest : public ::testing::Test { };
TYPED_TEST_CASE_P(Maps_InsertAndRemoveTest);

TYPED_TEST_P(Maps_InsertAndRemoveTest, InsertAndRemove)
{
  using TestValue = map_test::TestValueType<typename TypeParam::HoldsInt>;
  typename TypeParam::HoldsInt map;
  EXPECT_TRUE(map.empty());

  // Check our move assigns aren't trashing
  EXPECT_NO_THROW(map.insert("bob", 1));
  EXPECT_EQ(1, map.size());
  EXPECT_NO_THROW(map.insert(ep::KVP<typename TypeParam::HoldsInt::KeyType, int>("sarah", 2)));
  EXPECT_EQ(2, map.size());

  // Check our move assigns aren't trashing
  using namespace map_test;
  typename TypeParam::HoldsTestValue testMap;
  EXPECT_NO_THROW(testMap.insert("bill", TestValue(10)));
  EXPECT_EQ(1, testMap.size());
  EXPECT_EQ(1, TestValue::numInstances());

  {
    TestValue localInstance(99);
    EXPECT_NO_THROW(testMap.insert("nigel", localInstance));
    EXPECT_EQ(3, TestValue::numInstances());
    EXPECT_EQ(99, localInstance.instanceData);
    EXPECT_EQ(99, testMap["nigel"].instanceData);
  }
  EXPECT_EQ(2, TestValue::numInstances());
  EXPECT_EQ(2, testMap.size());

  // Lazy insert
  testMap.tryInsert("gordon", []() -> TestValue { return TestValue(101); });
  EXPECT_EQ(3, TestValue::numInstances());
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
  EXPECT_EQ(1, TestValue::numInstances());
  EXPECT_EQ(1, testMap.size());
  EXPECT_THROW(testMap.insert("bob", TestValue(11)), ep::EPException);
  EXPECT_EQ(10, testMap["bob"].instanceData);
  EXPECT_EQ(10, testMap.at("bob").instanceData);
  EXPECT_EQ(1, TestValue::numInstances());
  EXPECT_EQ(1, testMap.size());
  testMap.replace("bob", TestValue(12));
  EXPECT_EQ(12, testMap["bob"].instanceData);
  EXPECT_EQ(1, TestValue::numInstances());
  EXPECT_EQ(1, testMap.size());

  // Insert lazy when it already exists - shouldn't replace
  testMap.tryInsert("bob", []() -> TestValue { return TestValue(101); });
  EXPECT_EQ(12, testMap.get("bob")->instanceData);
  EXPECT_EQ(1, TestValue::numInstances());
  EXPECT_EQ(1, testMap.size());
}

template <typename T>
class Maps_Iterators : public ::testing::Test { };
TYPED_TEST_CASE_P(Maps_Iterators);

TYPED_TEST_P(Maps_Iterators, Iterators)
{
  using namespace map_test;
  using TestValue = TestValueType<typename TypeParam::HoldsInt>;
  typename TypeParam::HoldsTestValue smallMap = ConstructMap<typename TypeParam::HoldsTestValue>(8, 4);
//  HashMap<SharedString, TestValue> smallMap(8, 4);
  const int TABLE_SIZE = 8;
  ep::KVP<ep::SharedString, TestValue> keyTable[TABLE_SIZE] =
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
  EXPECT_EQ(TABLE_SIZE, (FindInstances<typename TypeParam::HoldsTestValue>(keyTable, TABLE_SIZE, smallMap)));

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

  EXPECT_EQ(TABLE_SIZE - 1, smallMap.size());

  // Attempt to find a key that doesn't exist
  EXPECT_EQ(smallMap.end(), smallMap.find("bob"));

  // Attempt to erase an invalid iterator
  EXPECT_ASSERT_THROW(smallMap.erase(smallMap.end()));

  // Insert past the proposed size and verify all our members exist
  //HashMap<SharedString, int> tinyMap(2);
  typename TypeParam::HoldsInt tinyMap = ConstructMap<typename TypeParam::HoldsInt>(2, 0x100);

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
  EXPECT_EQ(NUM_ENTRIES - 1, tinyMap.size());

  // Remove the last element
  firstIter = tinyMap.begin();
  nextIter = firstIter;
  while (++nextIter != tinyMap.end())
    firstIter = nextIter;

  EXPECT_EQ(nextIter, tinyMap.end());
  EXPECT_EQ(nextIter, tinyMap.erase(firstIter));
  EXPECT_EQ(NUM_ENTRIES - 2, tinyMap.size());

  // Iterate and remove each remaining element
  nextIter = tinyMap.begin();
  sum = 0;
  while (nextIter != tinyMap.end())
  {
    nextIter = tinyMap.erase(nextIter);
    ++sum;
  }

  EXPECT_TRUE(tinyMap.empty());
  EXPECT_EQ(NUM_ENTRIES - 2, sum);
}

REGISTER_TYPED_TEST_CASE_P(Maps_Coverage, Coverage);
REGISTER_TYPED_TEST_CASE_P(Maps_InsertAndRemoveTest, InsertAndRemove);
REGISTER_TYPED_TEST_CASE_P(Maps_Iterators, Iterators);

#endif// !defined(_EP_MAP_TEST_H)
