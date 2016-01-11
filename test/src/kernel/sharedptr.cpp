#include "gtest/gtest.h"
#include "ep/cpp/platform.h"
#include "ep/cpp/sharedptr.h"
#include "ep/cpp/safeptr.h"

// TODO: fill out these tests

class TestClass : public RefCounted
{
public:
  TestClass()
  {
    test = 10;
  }
  ~TestClass()
  {
    test = 100;
  }

  static int test;
};
int TestClass::test = 0;

// TODO: these tests are old and should be deprecated!!
TEST(EPSharedPtrTest, Deprecated)
{
  auto upT1 = UniquePtr<TestClass>(new TestClass);
  EPASSERT(TestClass::test == 10, "!");

  UniquePtr<TestClass> upT2 = nullptr;
  upT2 = upT1;
  EPASSERT(upT1 == nullptr, "UniquePtr should be null!");
  SharedPtr<TestClass> spT1 = upT2;
  EPASSERT(!upT2, "UniquePtr should be null!");
  EPASSERT(spT1 != nullptr, "SharedPtr should be null!");
  EPASSERT(spT1.count() == 1, "spT1 has only ref");

  SharedPtr<TestClass> spT2 = upT1;
  EPASSERT(spT2 == nullptr, "!");

  spT2 = spT1;
  EPASSERT(spT1 == spT2, "!");
  EPASSERT(spT2.count() == 2, "!");

  spT2 = nullptr;
  EPASSERT(!spT2, "!");
  EPASSERT(spT1.count() == 1, "!");

  SafePtr<TestClass> wpT1(spT1);
  EPASSERT(wpT1.ptr() == spT1.ptr(), "!");
  EPASSERT(spT1.count() == 1, "!");

  spT1 = nullptr;
  EPASSERT(wpT1 == nullptr, "!");

  wpT1 = nullptr;
  EPASSERT(!wpT1, "!");

  EPASSERT(TestClass::test == 100, "!");
}
