#include "eptest.h"
#include "traits/shared.h"
#include "ep/cpp/sharedptr.h"

using ep::RefCounted;
using ep::SharedPtr;
using ep::SafePtr;
using ep::UniquePtr;

namespace sharedptr_test {

const int TESTCLASS_CREATED = 10;
const int TESTCLASS_DESTROYED = 100;

class TestClass : public RefCounted
{
public:
  TestClass()
  {
    test = TESTCLASS_CREATED;
  }
  ~TestClass()
  {
    test = TESTCLASS_DESTROYED;
  }

  static int test;

  int instanceData = 0;
};
int TestClass::test = 0;


class NonRefCountedTestClass
{
public:
  NonRefCountedTestClass()
  {
    test = TESTCLASS_CREATED;
  }
  ~NonRefCountedTestClass()
  {
    test = TESTCLASS_DESTROYED;
  }

  static int test;
};
int NonRefCountedTestClass::test = 0;

};

// Traits

struct TestStruct
{
  using Type = SharedPtr<sharedptr_test::TestClass>;
  using ConstType = SharedPtr<const sharedptr_test::TestClass>;

  static SharedPtr<sharedptr_test::TestClass> create()
  {
    return SharedPtr<sharedptr_test::TestClass>::create();
  }
};

using MyTypes = typename ::testing::Types<TestStruct>;
INSTANTIATE_TYPED_TEST_CASE_P(SharedPtr_SharedTrait, Traits_Shared, MyTypes);


static_assert(ep::IsShared<TestStruct::Type>::value == true, "ep::IsShared failed!");
static_assert(ep::IsShared<TestStruct::ConstType>::value == true, "ep::IsShared failed!");


// ------------------------------- RefCounted Tests ----------------------------------------------

TEST(RefCounted, IncAndDec)
{
  RefCounted *pTestClass = RefCounted::create<sharedptr_test::TestClass>();
  ASSERT_TRUE(pTestClass);
  EXPECT_EQ(0, pTestClass->use_count());
  EXPECT_EQ(sharedptr_test::TESTCLASS_CREATED, sharedptr_test::TestClass::test);

  EXPECT_EQ(1, pTestClass->incRef());
  EXPECT_EQ(1, pTestClass->use_count());

  EXPECT_EQ(0, pTestClass->decRef());
  EXPECT_EQ(sharedptr_test::TESTCLASS_DESTROYED, sharedptr_test::TestClass::test);
}


// ------------------------------- SharedPtr Tests -----------------------------------------------

TEST(SharedPtr, CreateNullPointer)
{
  // default construction is null
  SharedPtr<sharedptr_test::TestClass> spDefault;
  EXPECT_TRUE(spDefault.get() == nullptr);

  // explicit null construction
  SharedPtr<sharedptr_test::TestClass> spNull = nullptr;
  EXPECT_TRUE(spNull.get() == nullptr);
}

TEST(SharedPtr, CreateAndDestroy)
{
  {
    // construct from create
    SharedPtr<sharedptr_test::TestClass> spTC = SharedPtr<sharedptr_test::TestClass>::create();
    ASSERT_TRUE(spTC.get() != nullptr);
    EXPECT_EQ(1, spTC.use_count());
    EXPECT_EQ(sharedptr_test::TESTCLASS_CREATED, sharedptr_test::TestClass::test);
  }
  EXPECT_EQ(sharedptr_test::TESTCLASS_DESTROYED, sharedptr_test::TestClass::test);
}

TEST(SharedPtr, CreateAndDestroyFromCopy)
{
  {
    SharedPtr<sharedptr_test::TestClass> spTC = SharedPtr<sharedptr_test::TestClass>::create();
    ASSERT_EQ(1, spTC.use_count());

    // copy construction
    SharedPtr<sharedptr_test::TestClass> spCopy = spTC;
    EXPECT_TRUE(spCopy.get() != nullptr);
    EXPECT_TRUE(spCopy.get() == spTC.get());
    EXPECT_EQ(2, spCopy.use_count());

    // copy construction to const TestClass
    SharedPtr<const sharedptr_test::TestClass> spCopyConst = spCopy;
    EXPECT_TRUE(spCopyConst.get() != nullptr);
    EXPECT_TRUE(spCopyConst.get() == spCopy.get());
    EXPECT_EQ(3, spTC.use_count());

    EXPECT_EQ(sharedptr_test::TESTCLASS_CREATED, sharedptr_test::TestClass::test);
  }
  EXPECT_EQ(sharedptr_test::TESTCLASS_DESTROYED, sharedptr_test::TestClass::test);
}

TEST(SharedPtr, CreateAndDestroyFromMove)
{
  {
    SharedPtr<sharedptr_test::TestClass> spTC = SharedPtr<sharedptr_test::TestClass>::create();
    ASSERT_EQ(1, spTC.use_count());

    sharedptr_test::TestClass *pTest = spTC.get();

    // move construction - following this block spTC is discarded and spMove replaces it by pointing to TestClass
    SharedPtr<sharedptr_test::TestClass> spMove = std::move(spTC);
    EXPECT_TRUE(spTC.get() == nullptr);
    EXPECT_TRUE(spMove.get() != nullptr);
    EXPECT_TRUE(spMove.get() == pTest);
    EXPECT_EQ(1, spMove.use_count());
    EXPECT_EQ(sharedptr_test::TESTCLASS_CREATED, sharedptr_test::TestClass::test);

    // move construction to const TestClass - this will discard spMove
    SharedPtr<const sharedptr_test::TestClass> spMoveConst = std::move(spMove);
    EXPECT_TRUE(spMove.get() == nullptr);
    EXPECT_TRUE(spMoveConst.get() != nullptr);
    EXPECT_TRUE(spMoveConst.get() == pTest);
    EXPECT_EQ(1, spMoveConst.use_count());
    EXPECT_EQ(sharedptr_test::TESTCLASS_CREATED, sharedptr_test::TestClass::test);
  }
  EXPECT_EQ(sharedptr_test::TESTCLASS_DESTROYED, sharedptr_test::TestClass::test);
}

TEST(SharedPtr, CreateAndDestroyFromUnique)
{
  // create from unique
  {
    UniquePtr<sharedptr_test::TestClass> upTC = UniquePtr<sharedptr_test::TestClass>::create();
    ASSERT_TRUE(upTC.get() != nullptr);
    EXPECT_EQ(1, upTC->use_count());

    SharedPtr<sharedptr_test::TestClass> spFromUnique = upTC;
    EXPECT_TRUE(spFromUnique.get() != nullptr);
    EXPECT_TRUE(upTC.get() == nullptr);
    EXPECT_EQ(1, spFromUnique.use_count());
    EXPECT_EQ(sharedptr_test::TESTCLASS_CREATED, sharedptr_test::TestClass::test);
  }
  EXPECT_EQ(sharedptr_test::TESTCLASS_DESTROYED, sharedptr_test::TestClass::test);

  // move from unique
  {
    UniquePtr<sharedptr_test::TestClass> upTC = UniquePtr<sharedptr_test::TestClass>::create();
    ASSERT_TRUE(upTC.get() != nullptr);
    EXPECT_EQ(1, upTC->use_count());

    SharedPtr<sharedptr_test::TestClass> spMoveFromUnique = std::move(upTC);
    EXPECT_TRUE(spMoveFromUnique.get() != nullptr);
    EXPECT_TRUE(upTC.get() == nullptr);
    EXPECT_EQ(1, spMoveFromUnique.use_count());
    EXPECT_EQ(sharedptr_test::TESTCLASS_CREATED, sharedptr_test::TestClass::test);
  }
  EXPECT_EQ(sharedptr_test::TESTCLASS_DESTROYED, sharedptr_test::TestClass::test);
}

TEST(SharedPtr, CreateAndDestroyFromRefCountedPointer)
{
  {
    sharedptr_test::TestClass *pTestClass = RefCounted::create<sharedptr_test::TestClass>();
    ASSERT_TRUE(pTestClass != nullptr);
    EXPECT_EQ(0, pTestClass->use_count());

    // construct from RefCounted ptr
    SharedPtr<sharedptr_test::TestClass> spFromPtr(pTestClass);
    EXPECT_TRUE(pTestClass == spFromPtr.get());
    EXPECT_EQ(1, spFromPtr.use_count());
    EXPECT_EQ(sharedptr_test::TESTCLASS_CREATED, sharedptr_test::TestClass::test);
  }
  EXPECT_EQ(sharedptr_test::TESTCLASS_DESTROYED, sharedptr_test::TestClass::test);
}

TEST(SharedPtr, CreateAndDestroyFromSafe)
{
  {
    sharedptr_test::TestClass *pTestClass = RefCounted::create<sharedptr_test::TestClass>();
    SafePtr<sharedptr_test::TestClass> sfTC(pTestClass);
    ASSERT_TRUE(sfTC.get() != nullptr);
    EXPECT_TRUE(sfTC.get() == pTestClass);
    EXPECT_EQ(0, sfTC->use_count());

    // construct from safeptr
    SharedPtr<sharedptr_test::TestClass> spFromSafe = sfTC;
    EXPECT_TRUE(spFromSafe.get() == sfTC.get());
    EXPECT_EQ(1, spFromSafe.use_count());
    EXPECT_EQ(sharedptr_test::TESTCLASS_CREATED, sharedptr_test::TestClass::test);
  }
  EXPECT_EQ(sharedptr_test::TESTCLASS_DESTROYED, sharedptr_test::TestClass::test);
}

TEST(SharedPtr, Reset)
{
  SharedPtr<sharedptr_test::TestClass> spTest = SharedPtr<sharedptr_test::TestClass>::create();
  EXPECT_EQ(1, spTest.use_count());
  EXPECT_TRUE(spTest);  // Bool test
  spTest.reset();
  EXPECT_EQ(0, spTest.use_count());
  EXPECT_TRUE(!spTest); // Bool test
}

TEST(SharedPtr, Deref)
{
  sharedptr_test::TestClass *pTestClass = RefCounted::create<sharedptr_test::TestClass>();
  SharedPtr<sharedptr_test::TestClass> spTest(pTestClass);
  EXPECT_EQ(pTestClass, spTest.operator->());
  EXPECT_EQ(pTestClass, spTest.get());
  pTestClass->instanceData = 15;
  EXPECT_EQ(pTestClass->instanceData, (*spTest).instanceData);
}

TEST(SharedPtr, Assignment)
{
  SharedPtr<sharedptr_test::TestClass> spTest1 = SharedPtr<sharedptr_test::TestClass>::create();
  SharedPtr<sharedptr_test::TestClass> spTest2 = nullptr;

  // assignment
  ASSERT_TRUE(spTest1.get() != nullptr);
  ASSERT_TRUE(spTest2.get() == nullptr);
  EXPECT_EQ(1, spTest1.use_count());
  spTest2 = spTest1;
  EXPECT_TRUE(spTest2.get() != nullptr);
  EXPECT_TRUE(spTest1.get() == spTest2.get());
  EXPECT_EQ(2, spTest1.use_count());
  EXPECT_EQ(sharedptr_test::TESTCLASS_CREATED, sharedptr_test::TestClass::test);

  // assignment to null
  spTest2 = nullptr;
  EXPECT_TRUE(spTest2.get() == nullptr);
  EXPECT_EQ(1, spTest1.use_count());
  EXPECT_EQ(sharedptr_test::TESTCLASS_CREATED, sharedptr_test::TestClass::test);

  // assignment from self shouldn't modify the use_count
  spTest1 = spTest1;
  EXPECT_EQ(1, spTest1.use_count());
  EXPECT_EQ(sharedptr_test::TESTCLASS_CREATED, sharedptr_test::TestClass::test);

  spTest1 = nullptr;
  EXPECT_TRUE(spTest1.get() == nullptr);
  EXPECT_EQ(sharedptr_test::TESTCLASS_DESTROYED, sharedptr_test::TestClass::test);
}

TEST(SharedPtr, MoveAssignment)
{
  SharedPtr<sharedptr_test::TestClass> spTest = SharedPtr<sharedptr_test::TestClass>::create();
  SharedPtr<sharedptr_test::TestClass> spMove = nullptr;

  ASSERT_TRUE(spTest.get() != nullptr);
  EXPECT_EQ(1, spTest.use_count());

  spMove = std::move(spTest);
  ASSERT_TRUE(spTest.get() == nullptr);
  EXPECT_TRUE(spMove.get() != nullptr);
  EXPECT_EQ(1, spMove.use_count());

  // move assignment from self shouldn't modify the use_count
  spMove = std::move(spMove);
  EXPECT_TRUE(spMove.get() != nullptr);
  EXPECT_EQ(1, spMove.use_count());

  spMove = nullptr;

  EXPECT_EQ(sharedptr_test::TESTCLASS_DESTROYED, sharedptr_test::TestClass::test);
}

TEST(SharedPtr, AssignmentToConst)
{
  // assignment to const data
  SharedPtr<sharedptr_test::TestClass> spTest = SharedPtr<sharedptr_test::TestClass>::create();
  SharedPtr<const sharedptr_test::TestClass> spTestConst = nullptr;

  spTestConst = spTest;
  EXPECT_EQ(2, spTest.use_count());
  EXPECT_TRUE(spTestConst.get() == spTest.get());

  spTestConst = nullptr;
  spTest = nullptr;

  EXPECT_EQ(sharedptr_test::TESTCLASS_DESTROYED, sharedptr_test::TestClass::test);
}

TEST(SharedPtr, AssignmentFromUnique)
{
  // assignment from UniquePtr
  const int MAGIC_NUMBER_UNIQUE = 222;
  UniquePtr<sharedptr_test::TestClass> uniquePtr = UniquePtr<sharedptr_test::TestClass>::create();
  SharedPtr<sharedptr_test::TestClass> spTest = nullptr;

  uniquePtr->instanceData = MAGIC_NUMBER_UNIQUE;
  spTest = uniquePtr;
  EXPECT_TRUE(uniquePtr.get() == nullptr);
  ASSERT_TRUE(spTest.get() != nullptr);
  EXPECT_EQ(MAGIC_NUMBER_UNIQUE, spTest->instanceData);
  EXPECT_EQ(1, spTest.use_count());

  // move assignment from UniquePtr
  const int MAGIC_NUMBER_UNIQUE2 = 224;
  UniquePtr<sharedptr_test::TestClass> uniquePtr2 = UniquePtr<sharedptr_test::TestClass>::create();
  uniquePtr2->instanceData = MAGIC_NUMBER_UNIQUE2;
  spTest = std::move(uniquePtr2);
  EXPECT_TRUE(uniquePtr2.get() == nullptr);
  ASSERT_TRUE(spTest.get() != nullptr);
  EXPECT_EQ(MAGIC_NUMBER_UNIQUE2, spTest->instanceData);
  EXPECT_EQ(1, spTest.use_count());

  spTest = nullptr;
  EXPECT_EQ(sharedptr_test::TESTCLASS_DESTROYED, sharedptr_test::TestClass::test);
}

TEST(SharedPtr, AssignmentFromSafe)
{
  // assignment from SafePtr
  SafePtr<sharedptr_test::TestClass> safePtr(RefCounted::create<sharedptr_test::TestClass>());
  SharedPtr<sharedptr_test::TestClass> spTest = nullptr;

  spTest = safePtr;
  EXPECT_TRUE(spTest.get() == safePtr.get());
  EXPECT_EQ(1, spTest.use_count());

  spTest = nullptr;
  EXPECT_EQ(sharedptr_test::TESTCLASS_DESTROYED, sharedptr_test::TestClass::test);
}

TEST(SharedPtr, ComparisonOperators)
{
  SharedPtr<sharedptr_test::TestClass> A = SharedPtr<sharedptr_test::TestClass>::create();
  SharedPtr<sharedptr_test::TestClass> B = nullptr;

  EXPECT_TRUE(A != B);
  EXPECT_FALSE(A == B);
  EXPECT_TRUE(B == nullptr);
  EXPECT_TRUE(nullptr == B);
  EXPECT_FALSE(A == nullptr);
  EXPECT_FALSE(nullptr == A);
  EXPECT_TRUE(A != nullptr);
  EXPECT_TRUE(nullptr != A);
  EXPECT_FALSE(B != nullptr);
  EXPECT_FALSE(nullptr != B);

  SharedPtr<sharedptr_test::TestClass> C = A;
  EXPECT_TRUE(C == A);
  EXPECT_TRUE(C >= A);
  EXPECT_TRUE(C <= A);
  EXPECT_FALSE(C > A);
  EXPECT_FALSE(C < A);

  // Since we can't guarantee what the addresses of the shared pointers will be at test runtime we need to check both cases
  SharedPtr<sharedptr_test::TestClass> D = SharedPtr<sharedptr_test::TestClass>::create();
  if (A.get() > D.get())
  {
    EXPECT_TRUE(A > D);
    EXPECT_TRUE(A >= D);
    EXPECT_FALSE(A < D);
    EXPECT_FALSE(A <= D);
  }
  else
  {
    EXPECT_TRUE(D > A);
    EXPECT_TRUE(D >= A);
    EXPECT_FALSE(D < A);
    EXPECT_FALSE(D <= A);
  }
}


// ------------------------------- UniquePtr Tests -----------------------------------------------

TEST(UniquePtr, CreateNullPointer)
{
  // default construction is null
  UniquePtr<sharedptr_test::TestClass> upDefault;
  EXPECT_TRUE(!upDefault);
  EXPECT_TRUE(upDefault.get() == nullptr);

  // explicit null construction
  UniquePtr<sharedptr_test::TestClass> upNull = nullptr;
  EXPECT_TRUE(upNull.get() == nullptr);
}

TEST(UniquePtr, CreateAndDestroy)
{
  {
    // construct from create
    UniquePtr<sharedptr_test::TestClass> upTC = UniquePtr<sharedptr_test::TestClass>::create();
    ASSERT_TRUE(upTC.get() != nullptr);
    EXPECT_EQ(1, upTC->use_count());
    EXPECT_EQ(sharedptr_test::TESTCLASS_CREATED, sharedptr_test::TestClass::test);
  }
  EXPECT_EQ(sharedptr_test::TESTCLASS_DESTROYED, sharedptr_test::TestClass::test);
}

TEST(UniquePtr, CreateAndDestroyConst)
{
  {
    // construct from create
    UniquePtr<const sharedptr_test::TestClass> upTC = UniquePtr<const sharedptr_test::TestClass>::create();
    ASSERT_TRUE(upTC.get() != nullptr);
    EXPECT_EQ(1, upTC->use_count());
    EXPECT_EQ(sharedptr_test::TESTCLASS_CREATED, sharedptr_test::TestClass::test);
  }
  EXPECT_EQ(sharedptr_test::TESTCLASS_DESTROYED, sharedptr_test::TestClass::test);
}

TEST(UniquePtr, CreateAndDestroyFromCopy)
{
  UniquePtr<sharedptr_test::TestClass> upTC = UniquePtr<sharedptr_test::TestClass>::create();
  ASSERT_TRUE(upTC.get() != nullptr);

  {
    // copy construction - this should reset the original unique pointer
    UniquePtr<sharedptr_test::TestClass> upCopy = std::move(upTC);
    EXPECT_TRUE(upCopy.get() != nullptr);
    EXPECT_TRUE(upTC.get() == nullptr);
    EXPECT_EQ(1, upCopy->use_count());
    EXPECT_EQ(sharedptr_test::TESTCLASS_CREATED, sharedptr_test::TestClass::test);

    // copy construction to const TestClass
    UniquePtr<const sharedptr_test::TestClass> upCopyConst = std::move(upCopy);
    EXPECT_TRUE(upCopyConst.get() != nullptr);
    EXPECT_TRUE(upCopy.get() == nullptr);
    EXPECT_EQ(1, upCopyConst->use_count());
    EXPECT_EQ(sharedptr_test::TESTCLASS_CREATED, sharedptr_test::TestClass::test);
  }
  EXPECT_EQ(sharedptr_test::TESTCLASS_DESTROYED, sharedptr_test::TestClass::test);
}

TEST(UniquePtr, CreateAndDestroyFromMove)
{
  {
    UniquePtr<sharedptr_test::TestClass> upTC = UniquePtr<sharedptr_test::TestClass>::create();
    ASSERT_TRUE(upTC.get() != nullptr);

    sharedptr_test::TestClass *pTest = upTC.get();

    // move construction
    UniquePtr<sharedptr_test::TestClass> upMove = std::move(upTC);
    EXPECT_TRUE(upTC.get() == nullptr);
    EXPECT_TRUE(upMove.get() != nullptr);
    EXPECT_TRUE(upMove.get() == pTest);
    EXPECT_EQ(1, upMove->use_count());
    EXPECT_EQ(sharedptr_test::TESTCLASS_CREATED, sharedptr_test::TestClass::test);

    // move construction to const TestClass - this will discard upMove
    UniquePtr<const sharedptr_test::TestClass> upMoveConst = std::move(upMove);
    EXPECT_TRUE(upMove.get() == nullptr);
    EXPECT_TRUE(upMoveConst.get() == pTest);
    EXPECT_EQ(1, upMoveConst->use_count());
    EXPECT_EQ(sharedptr_test::TESTCLASS_CREATED, sharedptr_test::TestClass::test);
  }
  EXPECT_EQ(sharedptr_test::TESTCLASS_DESTROYED, sharedptr_test::TestClass::test);
}

TEST(UniquePtr, CreateAndDestroyFromRefCountedPointer)
{
  sharedptr_test::TestClass *pTestClass = RefCounted::create<sharedptr_test::TestClass>();
  UniquePtr<sharedptr_test::TestClass> upFromPtr(pTestClass);
  ASSERT_TRUE(pTestClass != nullptr);
  ASSERT_TRUE(upFromPtr.get() != nullptr);
  EXPECT_EQ(pTestClass, upFromPtr.get());
  EXPECT_EQ(1, pTestClass->use_count());

  SharedPtr<sharedptr_test::TestClass> spTest = upFromPtr;
  EXPECT_TRUE(upFromPtr.get() == nullptr);
  ASSERT_TRUE(spTest.get() != nullptr);
  EXPECT_EQ(pTestClass, spTest.get());
  EXPECT_EQ(1, pTestClass->use_count());
}

TEST(UniquePtr, CreateAndDestroyFromNonRefCountedPointer)
{
  UniquePtr<sharedptr_test::NonRefCountedTestClass> upFromPtr = UniquePtr<sharedptr_test::NonRefCountedTestClass>::create();
  ASSERT_TRUE(upFromPtr.get() != nullptr);
  EXPECT_EQ(sharedptr_test::TESTCLASS_CREATED, sharedptr_test::NonRefCountedTestClass::test);
  upFromPtr = nullptr;
  EXPECT_EQ(sharedptr_test::TESTCLASS_DESTROYED, sharedptr_test::NonRefCountedTestClass::test);
}

TEST(UniquePtr, CreateAndDestroyFromNonRefCountedPointerConst)
{
  UniquePtr<const sharedptr_test::NonRefCountedTestClass> upFromPtr = UniquePtr<const sharedptr_test::NonRefCountedTestClass>::create();
  ASSERT_TRUE(upFromPtr.get() != nullptr);
  EXPECT_EQ(sharedptr_test::TESTCLASS_CREATED, sharedptr_test::NonRefCountedTestClass::test);
  upFromPtr = nullptr;
  EXPECT_EQ(sharedptr_test::TESTCLASS_DESTROYED, sharedptr_test::NonRefCountedTestClass::test);
}

TEST(UniquePtr, Reset)
{
  UniquePtr<sharedptr_test::TestClass> upTest = UniquePtr<sharedptr_test::TestClass>::create();
  ASSERT_TRUE(upTest.get() != nullptr);
  EXPECT_TRUE(upTest);  // Bool test
  upTest.reset();
  EXPECT_TRUE(upTest.get() == nullptr);
  EXPECT_TRUE(!upTest); // Bool test
}

TEST(UniquePtr, Deref)
{
  sharedptr_test::TestClass *pTestClass = RefCounted::create<sharedptr_test::TestClass>();
  UniquePtr<sharedptr_test::TestClass> upTest(pTestClass);
  EXPECT_EQ(pTestClass, upTest.operator->());
  EXPECT_EQ(pTestClass, upTest.get());
  pTestClass->instanceData = 15;
  EXPECT_EQ(pTestClass->instanceData, (*upTest).instanceData);
}

TEST(UniquePtr, Assignment)
{
  UniquePtr<sharedptr_test::TestClass> upTest1 = UniquePtr<sharedptr_test::TestClass>::create();
  UniquePtr<sharedptr_test::TestClass> upTest2 = nullptr;

  // assignment
  ASSERT_TRUE(upTest1.get() != nullptr);
  ASSERT_TRUE(upTest2.get() == nullptr);
  EXPECT_EQ(sharedptr_test::TESTCLASS_CREATED, sharedptr_test::TestClass::test);
  upTest2 = std::move(upTest1);
  EXPECT_TRUE(upTest1.get() == nullptr);
  EXPECT_TRUE(upTest2.get() != nullptr);
  EXPECT_EQ(1, upTest2->use_count());
  EXPECT_EQ(sharedptr_test::TESTCLASS_CREATED, sharedptr_test::TestClass::test);

  // assignment to self should not kill or modify the pointer/use_count
  upTest2 = std::move(upTest2);
  EXPECT_TRUE(upTest2 != nullptr);
  EXPECT_EQ(1, upTest2->use_count());

  // assignment to SharedPtr
  SharedPtr<sharedptr_test::TestClass> spTest = upTest2;
  EXPECT_TRUE(spTest.get() != nullptr);
  EXPECT_TRUE(upTest2.get() == nullptr);
  EXPECT_EQ(1, spTest.use_count());
  EXPECT_EQ(sharedptr_test::TESTCLASS_CREATED, sharedptr_test::TestClass::test);

  spTest = nullptr;
  EXPECT_EQ(sharedptr_test::TESTCLASS_DESTROYED, sharedptr_test::TestClass::test);
}

TEST(UniquePtr, MoveAssignment)
{
  UniquePtr<sharedptr_test::TestClass> upTest = UniquePtr<sharedptr_test::TestClass>::create();
  UniquePtr<sharedptr_test::TestClass> upMove = nullptr;
  RefCounted *pRef = upTest.get();

  ASSERT_TRUE(upTest.get() != nullptr);

  upMove = std::move(upTest);
  ASSERT_TRUE(upTest.get() == nullptr);
  EXPECT_TRUE(upMove.get() != nullptr);
  EXPECT_TRUE(upMove.get() == pRef);

  // move assignment to self should not kill or modify the pointer/use_count
  upMove = std::move(upMove);
  EXPECT_TRUE(upMove.get() != nullptr);
  EXPECT_EQ(1, upMove->use_count());
}

TEST(UniquePtr, ComparisonOperators)
{
  UniquePtr<sharedptr_test::TestClass> A = UniquePtr<sharedptr_test::TestClass>::create();
  UniquePtr<sharedptr_test::TestClass> B = nullptr;

  EXPECT_TRUE(A != B);
  EXPECT_FALSE(A == B);
  EXPECT_TRUE(B == nullptr);
  EXPECT_TRUE(nullptr == B);
  EXPECT_FALSE(A == nullptr);
  EXPECT_FALSE(nullptr == A);
  EXPECT_TRUE(A != nullptr);
  EXPECT_TRUE(nullptr != A);
  EXPECT_FALSE(B != nullptr);
  EXPECT_FALSE(nullptr != B);

  EXPECT_TRUE(A == A);
  EXPECT_TRUE(A >= A);
  EXPECT_TRUE(A <= A);

  // Since we can't guarantee what the addresses of the shared pointers will be at test runtime we need to check both cases
  UniquePtr<sharedptr_test::TestClass> C = UniquePtr<sharedptr_test::TestClass>::create();
  if (A.get() > C.get())
  {
    EXPECT_TRUE(A > C);
    EXPECT_TRUE(A >= C);
    EXPECT_FALSE(A < C);
    EXPECT_FALSE(A <= C);
  }
  else
  {
    EXPECT_TRUE(C > A);
    EXPECT_TRUE(C >= A);
    EXPECT_FALSE(C < A);
    EXPECT_FALSE(C <= A);
  }
}
