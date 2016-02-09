#include "gtest/gtest.h"
#include "ep/cpp/sharedptr.h"


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


// ------------------------------- RefCounted Tests ----------------------------------------------

TEST(RefCounted, IncAndDec)
{
  RefCounted *pTestClass = RefCounted::New<sharedptr_test::TestClass>();
  ASSERT_TRUE(pTestClass);
  EXPECT_EQ(0, pTestClass->RefCount());
  EXPECT_EQ(sharedptr_test::TESTCLASS_CREATED, sharedptr_test::TestClass::test);

  EXPECT_EQ(1, pTestClass->IncRef());
  EXPECT_EQ(1, pTestClass->RefCount());

  EXPECT_EQ(0, pTestClass->DecRef());
  EXPECT_EQ(sharedptr_test::TESTCLASS_DESTROYED, sharedptr_test::TestClass::test);
}


// ------------------------------- SharedPtr Tests -----------------------------------------------

TEST(SharedPtr, CreateNullPointer)
{
  // default construction is null
  SharedPtr<sharedptr_test::TestClass> spDefault;
  EXPECT_TRUE(!spDefault);
  EXPECT_TRUE(spDefault.ptr() == nullptr);
  EXPECT_EQ(0, spDefault.count());

  // explicit null construction
  SharedPtr<sharedptr_test::TestClass> spNull = nullptr;
  EXPECT_TRUE(spNull.ptr() == nullptr);
  EXPECT_EQ(0, spNull.count());
}

TEST(SharedPtr, CreateAndDestroy)
{
  {
    // construct from create
    SharedPtr<sharedptr_test::TestClass> spTC = SharedPtr<sharedptr_test::TestClass>::create();
    ASSERT_TRUE(spTC.ptr() != nullptr);
    EXPECT_EQ(1, spTC.count());
    EXPECT_EQ(sharedptr_test::TESTCLASS_CREATED, sharedptr_test::TestClass::test);
  }
  EXPECT_EQ(sharedptr_test::TESTCLASS_DESTROYED, sharedptr_test::TestClass::test);
}

TEST(SharedPtr, CreateAndDestroyFromCopy)
{
  {
    SharedPtr<sharedptr_test::TestClass> spTC = SharedPtr<sharedptr_test::TestClass>::create();
    ASSERT_EQ(1, spTC.count());

    // copy construction
    SharedPtr<sharedptr_test::TestClass> spCopy = spTC;
    EXPECT_TRUE(spCopy.ptr() != nullptr);
    EXPECT_TRUE(spCopy.ptr() == spTC.ptr());
    EXPECT_EQ(2, spCopy.count());

    // copy construction to const TestClass
    SharedPtr<const sharedptr_test::TestClass> spCopyConst = spCopy;
    EXPECT_TRUE(spCopyConst.ptr() != nullptr);
    EXPECT_TRUE(spCopyConst.ptr() == spCopy.ptr());
    EXPECT_EQ(3, spTC.count());

    EXPECT_EQ(sharedptr_test::TESTCLASS_CREATED, sharedptr_test::TestClass::test);
  }
  EXPECT_EQ(sharedptr_test::TESTCLASS_DESTROYED, sharedptr_test::TestClass::test);
}

TEST(SharedPtr, CreateAndDestroyFromMove)
{
  {
    SharedPtr<sharedptr_test::TestClass> spTC = SharedPtr<sharedptr_test::TestClass>::create();
    ASSERT_EQ(1, spTC.count());

    sharedptr_test::TestClass *pTest = spTC.ptr();

    // move construction - following this block spTC is discarded and spMove replaces it by pointing to TestClass
    SharedPtr<sharedptr_test::TestClass> spMove = std::move(spTC);
    EXPECT_TRUE(spTC.ptr() == nullptr);
    EXPECT_TRUE(spMove.ptr() != nullptr);
    EXPECT_TRUE(spMove.ptr() == pTest);
    EXPECT_EQ(1, spMove.count());
    EXPECT_EQ(sharedptr_test::TESTCLASS_CREATED, sharedptr_test::TestClass::test);

    // move construction to const TestClass - this will discard spMove
    SharedPtr<const sharedptr_test::TestClass> spMoveConst = std::move(spMove);
    EXPECT_TRUE(spMove.ptr() == nullptr);
    EXPECT_TRUE(spMoveConst.ptr() != nullptr);
    EXPECT_TRUE(spMoveConst.ptr() == pTest);
    EXPECT_EQ(1, spMoveConst.count());
    EXPECT_EQ(sharedptr_test::TESTCLASS_CREATED, sharedptr_test::TestClass::test);
  }
  EXPECT_EQ(sharedptr_test::TESTCLASS_DESTROYED, sharedptr_test::TestClass::test);
}

TEST(SharedPtr, CreateAndDestroyFromUnique)
{
  // create from unique
  {
    UniquePtr<sharedptr_test::TestClass> upTC = UniquePtr<sharedptr_test::TestClass>::create();
    ASSERT_TRUE(upTC.ptr() != nullptr);
    EXPECT_EQ(1, upTC->RefCount());

    SharedPtr<sharedptr_test::TestClass> spFromUnique = upTC;
    EXPECT_TRUE(spFromUnique.ptr() != nullptr);
    EXPECT_TRUE(upTC.ptr() == nullptr);
    EXPECT_EQ(1, spFromUnique.count());
    EXPECT_EQ(sharedptr_test::TESTCLASS_CREATED, sharedptr_test::TestClass::test);
  }
  EXPECT_EQ(sharedptr_test::TESTCLASS_DESTROYED, sharedptr_test::TestClass::test);

  // move from unique
  {
    UniquePtr<sharedptr_test::TestClass> upTC = UniquePtr<sharedptr_test::TestClass>::create();
    ASSERT_TRUE(upTC.ptr() != nullptr);
    EXPECT_EQ(1, upTC->RefCount());

    SharedPtr<sharedptr_test::TestClass> spMoveFromUnique = std::move(upTC);
    EXPECT_TRUE(spMoveFromUnique.ptr() != nullptr);
    EXPECT_TRUE(upTC.ptr() == nullptr);
    EXPECT_EQ(1, spMoveFromUnique.count());
    EXPECT_EQ(sharedptr_test::TESTCLASS_CREATED, sharedptr_test::TestClass::test);
  }
  EXPECT_EQ(sharedptr_test::TESTCLASS_DESTROYED, sharedptr_test::TestClass::test);
}

TEST(SharedPtr, CreateAndDestroyFromRefCountedPointer)
{
  {
    sharedptr_test::TestClass *pTestClass = RefCounted::New<sharedptr_test::TestClass>();
    ASSERT_TRUE(pTestClass != nullptr);
    EXPECT_EQ(0, pTestClass->RefCount());

    // construct from refcounted ptr
    SharedPtr<sharedptr_test::TestClass> spFromPtr(pTestClass);
    EXPECT_TRUE(pTestClass == spFromPtr.ptr());
    EXPECT_EQ(1, spFromPtr.count());
    EXPECT_EQ(sharedptr_test::TESTCLASS_CREATED, sharedptr_test::TestClass::test);
  }
  EXPECT_EQ(sharedptr_test::TESTCLASS_DESTROYED, sharedptr_test::TestClass::test);
}

TEST(SharedPtr, CreateAndDestroyFromSafe)
{
  {
    sharedptr_test::TestClass *pTestClass = RefCounted::New<sharedptr_test::TestClass>();
    SafePtr<sharedptr_test::TestClass> sfTC(pTestClass);
    ASSERT_TRUE(sfTC.ptr() != nullptr);
    EXPECT_TRUE(sfTC.ptr() == pTestClass);
    EXPECT_EQ(0, sfTC->RefCount());

    // construct from safeptr
    SharedPtr<sharedptr_test::TestClass> spFromSafe = sfTC;
    EXPECT_TRUE(spFromSafe.ptr() == sfTC.ptr());
    EXPECT_EQ(1, spFromSafe.count());
    EXPECT_EQ(sharedptr_test::TESTCLASS_CREATED, sharedptr_test::TestClass::test);
  }
  EXPECT_EQ(sharedptr_test::TESTCLASS_DESTROYED, sharedptr_test::TestClass::test);
}

TEST(SharedPtr, Reset)
{
  SharedPtr<sharedptr_test::TestClass> spTest = SharedPtr<sharedptr_test::TestClass>::create();
  EXPECT_EQ(1, spTest.count());
  EXPECT_TRUE(spTest);  // Bool test
  spTest.reset();
  EXPECT_EQ(0, spTest.count());
  EXPECT_TRUE(!spTest); // Bool test
}

TEST(SharedPtr, Unique)
{
  SharedPtr<sharedptr_test::TestClass> spTest = SharedPtr<sharedptr_test::TestClass>::create();
  EXPECT_EQ(1, spTest.count());
  EXPECT_EQ(true, spTest.unique());
  SharedPtr<sharedptr_test::TestClass> spTest2 = spTest;
  EXPECT_EQ(2, spTest.count());
  EXPECT_EQ(false, spTest.unique());
}

TEST(SharedPtr, Deref)
{
  sharedptr_test::TestClass *pTestClass = RefCounted::New<sharedptr_test::TestClass>();
  SharedPtr<sharedptr_test::TestClass> spTest(pTestClass);
  EXPECT_EQ(pTestClass, spTest.operator->());
  EXPECT_EQ(pTestClass, spTest.ptr());
  pTestClass->instanceData = 15;
  EXPECT_EQ(pTestClass->instanceData, (*spTest).instanceData);
}

TEST(SharedPtr, Assignment)
{
  SharedPtr<sharedptr_test::TestClass> spTest1 = SharedPtr<sharedptr_test::TestClass>::create();
  SharedPtr<sharedptr_test::TestClass> spTest2 = nullptr;

  // assignment
  ASSERT_TRUE(spTest1.ptr() != nullptr);
  ASSERT_TRUE(spTest2.ptr() == nullptr);
  EXPECT_EQ(1, spTest1.count());
  spTest2 = spTest1;
  EXPECT_TRUE(spTest2.ptr() != nullptr);
  EXPECT_TRUE(spTest1.ptr() == spTest2.ptr());
  EXPECT_EQ(2, spTest1.count());
  EXPECT_EQ(sharedptr_test::TESTCLASS_CREATED, sharedptr_test::TestClass::test);

  // assignment to null
  spTest2 = nullptr;
  EXPECT_TRUE(spTest2.ptr() == nullptr);
  EXPECT_EQ(1, spTest1.count());
  EXPECT_EQ(sharedptr_test::TESTCLASS_CREATED, sharedptr_test::TestClass::test);

  // assignment from self shouldn't modify the refcount
  spTest1 = spTest1;
  EXPECT_EQ(1, spTest1.count());
  EXPECT_EQ(sharedptr_test::TESTCLASS_CREATED, sharedptr_test::TestClass::test);

  spTest1 = nullptr;
  EXPECT_TRUE(spTest1.ptr() == nullptr);
  EXPECT_EQ(sharedptr_test::TESTCLASS_DESTROYED, sharedptr_test::TestClass::test);
}

TEST(SharedPtr, MoveAssignment)
{
  SharedPtr<sharedptr_test::TestClass> spTest = SharedPtr<sharedptr_test::TestClass>::create();
  SharedPtr<sharedptr_test::TestClass> spMove = nullptr;

  ASSERT_TRUE(spTest.ptr() != nullptr);
  EXPECT_EQ(1, spTest.count());

  spMove = std::move(spTest);
  ASSERT_TRUE(spTest.ptr() == nullptr);
  EXPECT_TRUE(spMove.ptr() != nullptr);
  EXPECT_EQ(1, spMove.count());

  // move assignment from self shouldn't modify the refcount
  spMove = std::move(spMove);
  EXPECT_TRUE(spMove.ptr() != nullptr);
  EXPECT_EQ(1, spMove.count());

  spMove = nullptr;

  EXPECT_EQ(sharedptr_test::TESTCLASS_DESTROYED, sharedptr_test::TestClass::test);
}

TEST(SharedPtr, AssignmentToConst)
{
  // assignment to const data
  SharedPtr<sharedptr_test::TestClass> spTest = SharedPtr<sharedptr_test::TestClass>::create();
  SharedPtr<const sharedptr_test::TestClass> spTestConst = nullptr;

  spTestConst = spTest;
  EXPECT_EQ(2, spTest.count());
  EXPECT_TRUE(spTestConst.ptr() == spTest.ptr());

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
  EXPECT_TRUE(uniquePtr.ptr() == nullptr);
  ASSERT_TRUE(spTest.ptr() != nullptr);
  EXPECT_EQ(MAGIC_NUMBER_UNIQUE, spTest->instanceData);
  EXPECT_EQ(1, spTest.count());

  // move assignment from UniquePtr
  const int MAGIC_NUMBER_UNIQUE2 = 224;
  UniquePtr<sharedptr_test::TestClass> uniquePtr2 = UniquePtr<sharedptr_test::TestClass>::create();
  uniquePtr2->instanceData = MAGIC_NUMBER_UNIQUE2;
  spTest = std::move(uniquePtr2);
  EXPECT_TRUE(uniquePtr2.ptr() == nullptr);
  ASSERT_TRUE(spTest.ptr() != nullptr);
  EXPECT_EQ(MAGIC_NUMBER_UNIQUE2, spTest->instanceData);
  EXPECT_EQ(1, spTest.count());

  spTest = nullptr;
  EXPECT_EQ(sharedptr_test::TESTCLASS_DESTROYED, sharedptr_test::TestClass::test);
}

TEST(SharedPtr, AssignmentFromSafe)
{
  // assignment from SafePtr
  SafePtr<sharedptr_test::TestClass> safePtr(RefCounted::New<sharedptr_test::TestClass>());
  SharedPtr<sharedptr_test::TestClass> spTest = nullptr;

  spTest = safePtr;
  EXPECT_TRUE(spTest.ptr() == safePtr.ptr());
  EXPECT_EQ(1, spTest.count());

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
  if (A.ptr() > D.ptr())
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
  EXPECT_TRUE(upDefault.ptr() == nullptr);

  // explicit null construction
  UniquePtr<sharedptr_test::TestClass> upNull = nullptr;
  EXPECT_TRUE(upNull.ptr() == nullptr);
}

TEST(UniquePtr, CreateAndDestroy)
{
  {
    // construct from create
    UniquePtr<sharedptr_test::TestClass> upTC = UniquePtr<sharedptr_test::TestClass>::create();
    ASSERT_TRUE(upTC.ptr() != nullptr);
    EXPECT_EQ(1, upTC->RefCount());
    EXPECT_EQ(sharedptr_test::TESTCLASS_CREATED, sharedptr_test::TestClass::test);
  }
  EXPECT_EQ(sharedptr_test::TESTCLASS_DESTROYED, sharedptr_test::TestClass::test);
}

TEST(UniquePtr, CreateAndDestroyFromCopy)
{
  UniquePtr<sharedptr_test::TestClass> upTC = UniquePtr<sharedptr_test::TestClass>::create();
  ASSERT_TRUE(upTC.ptr() != nullptr);

  {
    // copy construction - this should reset the original unique pointer
    UniquePtr<sharedptr_test::TestClass> upCopy = upTC;
    EXPECT_TRUE(upCopy.ptr() != nullptr);
    EXPECT_TRUE(upTC.ptr() == nullptr);
    EXPECT_EQ(1, upCopy->RefCount());
  }
  EXPECT_EQ(sharedptr_test::TESTCLASS_DESTROYED, sharedptr_test::TestClass::test);
}

TEST(UniquePtr, CreateAndDestroyFromMove)
{
  {
    UniquePtr<sharedptr_test::TestClass> upTC = UniquePtr<sharedptr_test::TestClass>::create();
    ASSERT_TRUE(upTC.ptr() != nullptr);

    sharedptr_test::TestClass *pTest = upTC.ptr();

    // move construction
    UniquePtr<sharedptr_test::TestClass> upMove = std::move(upTC);
    EXPECT_TRUE(upTC.ptr() == nullptr);
    EXPECT_TRUE(upMove.ptr() != nullptr);
    EXPECT_TRUE(upMove.ptr() == pTest);
    EXPECT_EQ(1, upMove->RefCount());
    EXPECT_EQ(sharedptr_test::TESTCLASS_CREATED, sharedptr_test::TestClass::test);
  }
  EXPECT_EQ(sharedptr_test::TESTCLASS_DESTROYED, sharedptr_test::TestClass::test);
}

TEST(UniquePtr, CreateAndDestroyFromRefCountedPointer)
{
  sharedptr_test::TestClass *pTestClass = RefCounted::New<sharedptr_test::TestClass>();
  UniquePtr<sharedptr_test::TestClass> upFromPtr(pTestClass);
  ASSERT_TRUE(pTestClass != nullptr);
  ASSERT_TRUE(upFromPtr.ptr() != nullptr);
  EXPECT_EQ(pTestClass, upFromPtr.ptr());
  EXPECT_EQ(1, pTestClass->RefCount());

  SharedPtr<sharedptr_test::TestClass> spTest = upFromPtr;
  EXPECT_TRUE(upFromPtr.ptr() == nullptr);
  ASSERT_TRUE(spTest.ptr() != nullptr);
  EXPECT_EQ(pTestClass, spTest.ptr());
  EXPECT_EQ(1, pTestClass->RefCount());
}

TEST(UniquePtr, CreateAndDestroyFromNonRefCountedPointer)
{
  UniquePtr<sharedptr_test::NonRefCountedTestClass> upFromPtr = UniquePtr<sharedptr_test::NonRefCountedTestClass>::create();
  ASSERT_TRUE(upFromPtr.ptr() != nullptr);
  EXPECT_EQ(sharedptr_test::TESTCLASS_CREATED, sharedptr_test::NonRefCountedTestClass::test);
  upFromPtr = nullptr;
  EXPECT_EQ(sharedptr_test::TESTCLASS_DESTROYED, sharedptr_test::NonRefCountedTestClass::test);
}

TEST(UniquePtr, Reset)
{
  UniquePtr<sharedptr_test::TestClass> upTest = UniquePtr<sharedptr_test::TestClass>::create();
  ASSERT_TRUE(upTest.ptr() != nullptr);
  EXPECT_TRUE(upTest);  // Bool test
  upTest.reset();
  EXPECT_TRUE(upTest.ptr() == nullptr);
  EXPECT_TRUE(!upTest); // Bool test
}

TEST(UniquePtr, Deref)
{
  sharedptr_test::TestClass *pTestClass = RefCounted::New<sharedptr_test::TestClass>();
  UniquePtr<sharedptr_test::TestClass> upTest(pTestClass);
  EXPECT_EQ(pTestClass, upTest.operator->());
  EXPECT_EQ(pTestClass, upTest.ptr());
  pTestClass->instanceData = 15;
  EXPECT_EQ(pTestClass->instanceData, (*upTest).instanceData);
}

TEST(UniquePtr, Assignment)
{
  UniquePtr<sharedptr_test::TestClass> upTest1 = UniquePtr<sharedptr_test::TestClass>::create();
  UniquePtr<sharedptr_test::TestClass> upTest2 = nullptr;

  // assignment
  ASSERT_TRUE(upTest1.ptr() != nullptr);
  ASSERT_TRUE(upTest2.ptr() == nullptr);
  EXPECT_EQ(sharedptr_test::TESTCLASS_CREATED, sharedptr_test::TestClass::test);
  upTest2 = upTest1;
  EXPECT_TRUE(upTest1.ptr() == nullptr);
  EXPECT_TRUE(upTest2.ptr() != nullptr);
  EXPECT_EQ(1, upTest2->RefCount());
  EXPECT_EQ(sharedptr_test::TESTCLASS_CREATED, sharedptr_test::TestClass::test);

  // assignment to self should not kill or modify the pointer/refcount
  upTest2 = upTest2;
  EXPECT_TRUE(upTest2 != nullptr);
  EXPECT_EQ(1, upTest2->RefCount());

  // assignment to SharedPtr
  SharedPtr<sharedptr_test::TestClass> spTest = upTest2;
  EXPECT_TRUE(spTest.ptr() != nullptr);
  EXPECT_TRUE(upTest2.ptr() == nullptr);
  EXPECT_EQ(1, spTest.count());
  EXPECT_EQ(sharedptr_test::TESTCLASS_CREATED, sharedptr_test::TestClass::test);

  spTest = nullptr;
  EXPECT_EQ(sharedptr_test::TESTCLASS_DESTROYED, sharedptr_test::TestClass::test);
}

TEST(UniquePtr, MoveAssignment)
{
  UniquePtr<sharedptr_test::TestClass> upTest = UniquePtr<sharedptr_test::TestClass>::create();
  UniquePtr<sharedptr_test::TestClass> upMove = nullptr;
  RefCounted *pRef = upTest.ptr();

  ASSERT_TRUE(upTest.ptr() != nullptr);

  upMove = std::move(upTest);
  ASSERT_TRUE(upTest.ptr() == nullptr);
  EXPECT_TRUE(upMove.ptr() != nullptr);
  EXPECT_TRUE(upMove.ptr() == pRef);

  // move assignment to self should not kill or modify the pointer/refcount
  upMove = std::move(upMove);
  EXPECT_TRUE(upMove.ptr() != nullptr);
  EXPECT_EQ(1, upMove->RefCount());
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
  if (A.ptr() > C.ptr())
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
