#include "eptest.h"
#include "traits.h"
#include "ep/cpp/platform.h"
// TODO: fill out these tests

using ep::Slice;
using ep::Array;
using ep::SharedArray;


static_assert(std::is_same<ep::IndexType<Slice<int>>, size_t>::value == true, "ep::IndexType failed!");
static_assert(std::is_same<ep::IndexType<Array<int>>, size_t>::value == true, "ep::IndexType failed!");
static_assert(std::is_same<ep::IndexType<SharedArray<int>>, size_t>::value == true, "ep::IndexType failed!");

static_assert(std::is_same<ep::ElementType<Slice<int>>, int>::value == true, "ep::ElementType failed!");
static_assert(std::is_same<ep::ElementType<Array<int>>, int>::value == true, "ep::ElementType failed!");
static_assert(std::is_same<ep::ElementType<SharedArray<int>>, int>::value == true, "ep::ElementType failed!");
static_assert(std::is_same<ep::ElementType<Slice<const int*>>, const int*>::value == true, "ep::ElementType failed!");
static_assert(std::is_same<ep::ElementType<Array<const int*>>, const int*>::value == true, "ep::ElementType failed!");
static_assert(std::is_same<ep::ElementType<SharedArray<const int*>>, const int*>::value == true, "ep::ElementType failed!");
static_assert(std::is_same<ep::ElementType<Slice<void>>, uint8_t>::value == true, "ep::ElementType failed!");

static_assert(ep::HasFront<Slice<int>>::value == true, "ep::HasFront failed!");
static_assert(ep::HasFront<Array<int>>::value == true, "ep::HasFront failed!");
static_assert(ep::HasFront<SharedArray<int>>::value == true, "ep::HasFront failed!");
static_assert(ep::HasBack<Slice<int>>::value == true, "ep::HasBack failed!");
static_assert(ep::HasBack<Array<int>>::value == true, "ep::HasBack failed!");
static_assert(ep::HasBack<SharedArray<int>>::value == true, "ep::HasBack failed!");

static_assert(ep::RandomAccessible<Slice<int>>::value == true, "ep::RandomAccessible failed!");
static_assert(ep::RandomAccessible<Array<int>>::value == true, "ep::RandomAccessible failed!");
static_assert(ep::RandomAccessible<SharedArray<int>>::value == true, "ep::RandomAccessible failed!");

static_assert(ep::HasSize<Slice<int>>::value == true, "ep::HasSize failed!");
static_assert(ep::HasSize<Array<int>>::value == true, "ep::HasSize failed!");
static_assert(ep::HasSize<SharedArray<int>>::value == true, "ep::HasSize failed!");

static_assert(ep::IsContainer<Array<int>>::value == true, "ep::IsContainer failed!");
static_assert(ep::IsContainer<SharedArray<int>>::value == true, "ep::IsContainer failed!");

static_assert(ep::Growable<Array<int>>::value == true, "ep::Growable failed!");
static_assert(ep::Growable<SharedArray<int>>::value == false, "ep::Growable failed!");
static_assert(ep::Shrinkable<Array<int>>::value == true, "ep::Shrinkable failed!");
static_assert(ep::Shrinkable<SharedArray<int>>::value == false, "ep::Shrinkable failed!");
static_assert(ep::IsMutable<Array<int>>::value == true, "ep::IsMutable failed!");
static_assert(ep::IsMutable<SharedArray<int>>::value == false, "ep::IsMutable failed!");

static_assert(ep::IsKeyed<Array<int>>::value == false, "ep::IsKeyed failed!");
static_assert(ep::IsKeyed<SharedArray<int>>::value == false, "ep::IsKeyed failed!");


// TODO: these tests are old and should be deprecated!!
TEST(EPSliceTest, Deprecated)
{
  // usSlice<> tests
  int i[100];

  Slice<int> i1(i, 100);        // slice from array
  Slice<const int> ci1(i, 100); // const slice from mutable array
  ci1 = i1;                       // assign mutable to const

  ci1 == i1;                      // compare pointer AND length are equal
  ci1 != i1;                      // compare pointer OR length are not equal

  i1.eq(ci1);                     // test elements for equality

  short s[100];
  Slice<short> s1(s, 100);

  i1.eq(s1);                      // test elements of different (compatible) types for equality (ie, int == short)

  auto slice = i1.slice(10, 20);  // 'slice' is a slice of i1 from elements [10, 20)  (exclusive: ie, [10, 19])
  slice[0];                       // element 0 from slice (ie, element 10 from i1)

  slice.empty();                  // ptr == nullptr || length == 0


  Slice<void> tttt = (Slice<void>)s1;
  tttt.ptr = tttt.ptr; // fixes gcc warning

                        // Array<>
  Array<int> s_i(i1);
  Array<const int> s_ci(s1);

  Slice<int> s_slice = s_i.slice(1, 3); // slices of Array are not owned; they die when the parent allocation dies
  s_slice.ptr = s_slice.ptr; // fixes gcc warning


                              // SharedArray<> tests
  SharedArray<int> rc_i1(i1);       // rc_i1 is an allocated, ref-counted copy of the slice i1
  SharedArray<const int> rc_ci1(s1);// rc_ci1 initialised from different (compatible) types (ie, short -> int, float -> double)

                                    // TODO: assign mutable RCSlice to const RCSlice without copy, should only bump RC

  auto rc_i2 = rc_i1;             // just inc the ref count

  auto rc_slice = rc_i1.slice(10, 20);  // slice increments the rc too

  rc_i1 = rc_slice;               // assignment between RC slices of the same array elide rc fiddling; only updates the offset/length

  i1 == rc_i2;                    // comparison of Slice and SharedArray; compares ptr && length as usual

  rc_i1.eq(i1);                   // element comparison between Slice and SharedArray

  for (auto j : i1)
  {
    j *= 1; // fixes gcc warning
            // iterate the elements in i1
            //...
  }
}
