#include "gtest/gtest.h"
#include "ep/cpp/delegate.h"

using ep::RefCounted;
using ep::Delegate;
using ep::SafePtr;
using ep::UniquePtr;

namespace delegate_test
{
const int k_magicNum1 = 111;
const int k_magicNum2 = 212;

int testMethod() { return k_magicNum1; }
void testEmptyMethod() {}
void testEmptyMethod2() {}

struct TestFunctor
{
  TestFunctor() {}
  int operator()() { return k_magicNum2; }
};

struct TestConstFunctor
{
  TestConstFunctor() {}
  int operator()() const { return k_magicNum2; }
};
};

TEST(Delegate, delgateTest)
{
  Delegate<int()> delegate(delegate_test::testMethod);

  EXPECT_EQ(delegate_test::testMethod(), delegate());
}

TEST(Delegate, delegateOperatorBool)
{
  Delegate<void()> emptyDelegate, delegateWithMethod(delegate_test::testEmptyMethod);

  EXPECT_FALSE(emptyDelegate);  // Check operator bool that it returns false when there is no method attached to this delegate
  EXPECT_TRUE(!emptyDelegate);  // Check operator bool with ! that it returns true when there is no method attached to this delegate

  EXPECT_TRUE(delegateWithMethod);   // Check operator bool that it returns true when there is a method attached to this delegate
  EXPECT_FALSE(!delegateWithMethod); // Check operator bool with ! that it returns false when there is a method attached to this delegate
}

TEST(Delegate, delegateEquality)
{
  Delegate<void()> delegateWithMethod1(delegate_test::testEmptyMethod),
                   delegateWithMethod1Copy(delegateWithMethod1),
                   delegateWithMethod2(delegate_test::testEmptyMethod),
                   delegateWithOtherMethod1(delegate_test::testEmptyMethod2),
                   emptyDelegate1, emptyDelegate2;


  EXPECT_FALSE(delegateWithMethod1 == delegateWithMethod2);       // Check == that it return false when 2 delegates have been constructed with the same method attached.
  EXPECT_TRUE(delegateWithMethod1 != delegateWithMethod2);        // Check != that it return false when 2 delegates have been constructed with the same method attached.

  EXPECT_FALSE(delegateWithMethod1 == delegateWithOtherMethod1);  // Check == that it return false when 2 delegates have different methods attached.
  EXPECT_TRUE(delegateWithMethod1 != delegateWithOtherMethod1);   // Check != that it return true when 2 delegates have different methods attached.

  EXPECT_TRUE(delegateWithMethod1 == delegateWithMethod1);        // Check == that it return true when comparing the same delegate.
  EXPECT_FALSE(delegateWithMethod1 != delegateWithMethod1);       // Check != that it return false when comparing the same delegate.

  EXPECT_TRUE(emptyDelegate1 == emptyDelegate1);                  // Check == that it return true when comparing the same empty delegate.
  EXPECT_FALSE(emptyDelegate1 != emptyDelegate1);                 // Check != that it return false when comparing the same empty delegate.

  EXPECT_TRUE(emptyDelegate1 == emptyDelegate2);                  // Check == that it return true when comparing different empty delegates.
  EXPECT_FALSE(emptyDelegate1 != emptyDelegate2);                 // Check != that it return false when comparing different empty delegates.

  EXPECT_TRUE(delegateWithMethod1 == delegateWithMethod1Copy);    // Check == that it return true when comparing a delegate copy.
  EXPECT_FALSE(delegateWithMethod1 != delegateWithMethod1Copy);   // Check != that it return false when comparing a delegate copy.
}

TEST(Delegate, functorDelegate)
{
  delegate_test::TestFunctor functor;
  delegate_test::TestConstFunctor constFunctor;
  const delegate_test::TestConstFunctor constFunctorObj;
  Delegate<int()> functorDelegate(functor), constFunctorDelegate(constFunctor), constObjFunctorDelegate(constFunctorObj);

  EXPECT_EQ(functorDelegate(), functor());                 // check that the delegate correctly called the functor and return the value correctly
  EXPECT_EQ(constFunctorDelegate(), constFunctor());       // check that the delegate correctly called the functor and return the value correctly
  EXPECT_EQ(constObjFunctorDelegate(), constFunctorObj()); // check that the delegate correctly called the functor and return the value correctly
}

TEST(Delegate, lambdaDelegate)
{
  int result = delegate_test::k_magicNum1;
  const auto constLambda = [&result]() { return result; };
  Delegate<int()> lambdaDelegate([&result]() { return result; }),
                  constLambdaDelegate(constLambda);

  result += delegate_test::k_magicNum1; // increment result

  EXPECT_EQ(result, lambdaDelegate()); // check that the delegate correctly called the lambda and return the captured value correctly
  EXPECT_EQ(result, constLambdaDelegate()); // check that the delegate correctly called the lambda and return the captured value correctly
}
