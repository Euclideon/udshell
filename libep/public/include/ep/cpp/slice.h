#pragma once
#if !defined(_EP_SLICE_HPP)
#define _EP_SLICE_HPP

#include <initializer_list>
#include <functional>

namespace ep {
namespace internal {

template<typename T> struct SliceElementType { typedef T Ty; };
template<> struct SliceElementType<void> { typedef uint8_t Ty; };
template<> struct SliceElementType<const void> { typedef const uint8_t Ty; };

struct SliceHeader
{
  typedef void (FreeFunc)(void *);
  FreeFunc *pFreeFunc;
  size_t allocatedCount; // TODO: allocCount and refCount are never used at the same time, make them a union?
  size_t refCount;       //       Array<> uses allocCount, SharedArray<> uses refCount
};

} // namespace internal


enum Concat_T  { Concat };
enum Alloc_T   { Alloc };
enum Reserve_T { Reserve };


// slices are bounded arrays, unlike C's conventional unbounded pointers (typically, with separate length stored in parallel)
// no attempt is made to create a one-size-fits-all implementation, as it is recognised that usages offer distinct advantages/disadvantages
// slice is the basis of the suite however, and everything is based on Slice. derived types address specifics in usage and/or ownership patterns

// declare an iterator so it works with standard range passed functions (foreach!)
template<typename T>
class Iterator
{
  // TODO: this could be made safer by storing a ref to the parent slice, and keeping an offset
  T *pI;

public:
  Iterator(T *pI) : pI(pI) {}
  bool operator!=(Iterator<T> rh) const { return pI != rh.pI; } // compare
  Iterator<T> operator++() { ++pI; return *this; }              // increment
  T& operator*() const { return *pI; }                            // value
  T *operator->() const { return pI; }
};

// Slice does not retain ownership of it's memory, it is used for temporary ownership; working locals, function args, etc
template<typename T>
struct Slice
{
public:
  using ET = typename internal::SliceElementType<T>::Ty;
  using ElementType = ET;

  size_t length;
  T *ptr;

  // constructors
  Slice();
  Slice(nullptr_t);
  explicit Slice(std::initializer_list<ET> list);
  Slice(T* ptr, size_t length);
  template<typename U> Slice(Slice<U> rh);
  template<typename U, size_t N> Slice(U(&arr)[N]);

  // assignment
  template<typename U> Slice<T>& operator =(Slice<U> rh);

  // contents
  ET& operator[](ptrdiff_t i) const;
  ET& at(ptrdiff_t i) const { return operator[](i); }

  Slice<T> slice(ptrdiff_t first, ptrdiff_t last) const;

  T* data() const { return ptr; }
  size_t size() const { return length; }
  bool empty() const;
  explicit operator bool() const { return length != 0; }

  // comparison
  bool operator ==(Slice<const T> rh) const;
  bool operator !=(Slice<const T> rh) const;

  template<typename U> bool eq(Slice<U> rh) const;
  template<typename U> ptrdiff_t cmp(Slice<U> rh) const;

  template<typename U> bool beginsWith(Slice<U> rh) const;
  template<typename U> bool endsWith(Slice<U> rh) const;

  // iterators
  Iterator<T> begin() const;
  Iterator<T> end() const;

  // useful functions
  ET& front() const;
  ET& back() const;
  void pop_front() { popFront(); }
  void pop_back() { popBack(); }

  ET& popFront();
  ET& popBack();
  Slice<T> pop(ptrdiff_t n);

  Slice<T> take(ptrdiff_t n) const;
  Slice<T> drop(ptrdiff_t n) const;

  bool exists(const ET &c, size_t *pIndex = nullptr) const;
  size_t findFirst(const ET &c) const;
  size_t findLast(const ET &c) const;
  template<typename U> size_t findFirst(Slice<U> s) const;
  template<typename U> size_t findLast(Slice<U> s) const;

  Slice<T> getLeftAtFirst(const ET &c, bool bInclusive = false) const;
  Slice<T> getLeftAtLast(const ET &c, bool bInclusive = false) const;
  Slice<T> getRightAtFirst(const ET &c, bool bInclusive = true) const;
  Slice<T> getRightAtLast(const ET &c, bool bInclusive = true) const;

  template<typename U> Slice<T> getLeftAtFirst(Slice<U> s, bool bInclusive = false) const;
  template<typename U> Slice<T> getLeftAtLast(Slice<U> s, bool bInclusive = false) const;
  template<typename U> Slice<T> getRightAtFirst(Slice<U> s, bool bInclusive = true) const;
  template<typename U> Slice<T> getRightAtLast(Slice<U> s, bool bInclusive = true) const;

  ptrdiff_t indexOfElement(const T *c) const;

  typedef bool(*Predicate)(const ET &e);
  T* search(Predicate) const;

  template<bool skipEmptyTokens = false>
  Slice<T> popToken(Slice<T> delimiters);

  template<bool skipEmptyTokens = false>
  Slice<Slice<T>> tokenise(Slice<Slice<T>> tokens, Slice<T> delimiters);
  template<bool skipEmptyTokens = false>
  size_t tokenise(std::function<void(Slice<T> token, size_t index)> onToken, Slice<T> delimiters);

  template<typename U>
  size_t copyTo(Slice<U> dest) const;
};
template<typename T>
inline bool operator ==(nullptr_t, Slice<T> rh) { return rh == nullptr; }
template<typename T>
inline bool operator !=(nullptr_t, Slice<T> rh) { return rh == nullptr; }

// Cast from a Slice<T> to a Slice<U> where T and U are compatible types.  Its typical use is for casting from a Slice<void>.
template <typename Dst, typename Src>
Dst slice_cast(Src src);

// Array introduces static-sized and/or stack-based ownership. this is useful anywhere that fixed-length arrays are appropriate
// Array will fail-over to an allocated buffer if the contents exceed the fixed size
template <typename T, size_t Count = 0>
struct Array : public Slice<T>
{
  // constructors
  Array();
  Array(nullptr_t);
  Array(Alloc_T, size_t count);
  Array(Reserve_T, size_t count);
  template <typename... Items> Array(Concat_T, Items&&... items);
  Array(std::initializer_list<T> list);
  Array(const Array<T, Count> &val);
  Array(Array<T, Count> &&rval);
  // TODO: Array copy/move constructors for const promotion?
  template <typename U> Array(U *ptr, size_t length);
  template <typename U> Array(Slice<U> slice);
  template <typename U, size_t N> Array(U(&arr)[N]);
  ~Array();

  void reserve(size_t count);
  void alloc(size_t count);
  void resize(size_t count);
  void clear();

  // assignment
  Array<T, Count>& operator =(const Array<T, Count> &rh);
  Array<T, Count>& operator =(Array<T, Count> &&rval);
  template <typename U> Array<T, Count>& operator =(Slice<U> rh);

  // manipulation
  template <typename... Things> Array<T, Count>& concat(Things&&... things);

  T& front() const;
  T& back() const;
  template <typename U> void push_front(U &&item);
  template <typename U> void push_back(U &&item);
  void pop_front();
  void pop_back();

  T& pushFront();
  T& pushBack();
  template <typename U> T& pushFront(U &&item);
  template <typename U> T& pushBack(U &&item);
  T popFront();
  T popBack();

  void remove(size_t i);
  void remove(const T *pItem);
  void removeFirst(const T &item) { remove(this->findFirst(item)); }

  void removeSwapLast(size_t i);
  void removeSwapLast(const T *pItem);
  void removeFirstSwapLast(const T &item) { removeSwapLast(this->findFirst(item)); }

  Slice<T> getBuffer() const;

private:
  friend struct Variant;
  template<typename U> friend struct SharedArray;

  template<size_t Len, bool = true>
  struct Buffer
  {
    char buffer[sizeof(T) * Len];
    bool hasAllocation(T *p) const { return p != (T*)buffer && p != nullptr; }
    T* ptr() const { return (T*)buffer; }
  };
  template<bool dummy> struct Buffer<0, dummy> // SORRY! VS2015 C99 still sucks; we must specialise here because can't zero-length-array! >_<
  {
    bool hasAllocation(T *p) const { return p != nullptr; }
    T* ptr() const { return nullptr; }
  };
  Buffer<Count> buffer;

  static size_t numToAlloc(size_t i);
  bool hasAllocation() const { return buffer.hasAllocation(this->ptr); }
};


// SharedArray is a reference counted slice, used to retain ownership of some memory, but will not duplicate when copies are made
// useful for long-living data that doesn't consume a fixed amount of their containing struct
// also useful for sharing between systems, passing between threads, etc.
// slices of SharedArray's increment the RC, so that slices can outlive the original owner, but without performing additional allocations and copies
template <typename T>
struct SharedArray : public Slice<T>
{
  // constructors
  SharedArray();
  SharedArray(nullptr_t);
  template <typename... Items> SharedArray(Concat_T, Items&&... items);
  SharedArray(std::initializer_list<typename SharedArray<T>::ET> list);
  SharedArray(const SharedArray<T> &rcslice);
  SharedArray(SharedArray<T> &&rval);
  template <typename U, size_t Len> SharedArray(const Array<U, Len> &arr);
  template <typename U, size_t Len> SharedArray(Array<U, Len> &&rval);
  template <typename U> SharedArray(U *ptr, size_t length);
  template <typename U> SharedArray(Slice<U> slice);
  template <typename U, size_t N> SharedArray(U(&arr)[N]);
  ~SharedArray();

  size_t use_count() const;

  Array<T> clone() { return Array<T>(*this); }

  // static constructors (make proper constructors?)
  template<typename... Things> static SharedArray<T> concat(Things&&... things);

  // assignment
  SharedArray<T>& operator =(const SharedArray<T> &rh);
  SharedArray<T>& operator =(SharedArray<T> &&rval);
  template <typename U> SharedArray<T>& operator =(Slice<U> rh);

private:
  void destroy();

  // hide a bunch of functions from slice
  void pop_front();
  void pop_back();
  ET& popFront();
  ET& popBack();
  Slice<T> pop(ptrdiff_t n);
};

} // namespace ep

#endif // _EP_SLICE_HPP
