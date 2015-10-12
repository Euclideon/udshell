#pragma once
#if !defined(_EP_SLICE)
#define _EP_SLICE

#include <initializer_list>

namespace ep {
namespace internal {

template<typename T> struct ElementType { typedef T Ty; };
template<> struct ElementType<void> { typedef uint8_t Ty; };
template<> struct ElementType<const void> { typedef const uint8_t Ty; };

} // namespace internal
} // namespace ep


struct epRC
{
  size_t refCount;
  size_t allocatedCount;
};


// slices are bounded arrays, unlike C's conventional unbounded pointers (typically, with separate length stored in parallel)
// no attempt is made to create a one-size-fits-all implementation, as it is recognised that usages offer distinct advantages/disadvantages
// slice is the basis of the suite however, and everything is based on epSlice. derived types address specifics in usage and/or ownership patterns

// declare an iterator so it works with standard range passed functions (foreach!)
template<typename T>
class epIterator
{
  // TODO: this could be made safer by storing a ref to the parent slice, and keeping an offset
  T *pI;

public:
  epIterator(T *pI) : pI(pI) {}
  bool operator!=(epIterator<T> rh) const { return pI != rh.pI; } // compare
  epIterator<T> operator++() { ++pI; return *this; }              // increment
  T& operator*() const { return *pI; }                            // value
};

// epSlice does not retain ownership of it's memory, it is used for temporary ownership; working locals, function args, etc
template<typename T>
struct epSlice
{
private:
public:
  typedef typename ep::internal::ElementType<T>::Ty ET;

  size_t length;
  T *ptr;

  // constructors
  epSlice<T>();
  epSlice<T>(nullptr_t);
  epSlice<T>(std::initializer_list<ET> list);
  epSlice<T>(T* ptr, size_t length);
  template<typename U> epSlice<T>(epSlice<U> rh);

  // assignment
  template<typename U> epSlice<T>& operator =(epSlice<U> rh);

  // contents
  ET& operator[](ptrdiff_t i) const;

  epSlice<T> slice(ptrdiff_t first, ptrdiff_t last) const;

  bool empty() const;
  explicit operator bool() const { return length != 0; }

  // comparison
  bool operator ==(epSlice<const T> rh) const;
  bool operator !=(epSlice<const T> rh) const;

  template<typename U> bool eq(epSlice<U> rh) const;
  template<typename U> ptrdiff_t cmp(epSlice<U> rh) const;

  template<typename U> bool beginsWith(epSlice<U> rh) const;
  template<typename U> bool endsWith(epSlice<U> rh) const;

  // iterators
  epIterator<T> begin() const;
  epIterator<T> end() const;

  // useful functions
  ET& front() const;
  ET& back() const;
  ET& popFront();
  ET& popBack();
  epSlice<T> get(ptrdiff_t n) const;
  epSlice<T> pop(ptrdiff_t n);
  epSlice<T> strip(ptrdiff_t n) const;

  bool exists(const ET &c, size_t *pIndex = nullptr) const;
  size_t findFirst(const ET &c) const;
  size_t findLast(const ET &c) const;
  template<typename U> size_t findFirst(epSlice<U> s) const;
  template<typename U> size_t findLast(epSlice<U> s) const;

  epSlice<T> getLeftAtFirst(const ET &c, bool bInclusive = false) const;
  epSlice<T> getLeftAtLast(const ET &c, bool bInclusive = false) const;
  epSlice<T> getRightAtFirst(const ET &c, bool bInclusive = true) const;
  epSlice<T> getRightAtLast(const ET &c, bool bInclusive = true) const;

  template<typename U> epSlice<T> getLeftAtFirst(epSlice<U> s, bool bInclusive = false) const;
  template<typename U> epSlice<T> getLeftAtLast(epSlice<U> s, bool bInclusive = false) const;
  template<typename U> epSlice<T> getRightAtFirst(epSlice<U> s, bool bInclusive = true) const;
  template<typename U> epSlice<T> getRightAtLast(epSlice<U> s, bool bInclusive = true) const;

  ptrdiff_t indexOfElement(const T *c) const;

  typedef bool(*Predicate)(const ET &e);
  T* search(Predicate) const;

  template<bool skipEmptyTokens = false>
  epSlice<T> popToken(epSlice<T> delimiters);

  template<bool skipEmptyTokens = false>
  epSlice<epSlice<T>> tokenise(epSlice<epSlice<T>> tokens, epSlice<T> delimiters);

  template<typename U>
  void copyTo(epSlice<U> dest) const;
};

// epArray introduces static-sized and/or stack-based ownership. this is useful anywhere that fixed-length arrays are appropriate
// epArray will fail-over to an allocated buffer if the contents exceed the fixed size
template <typename T, size_t Count = 0>
struct epArray : public epSlice<T>
{
  // constructors
  epArray<T, Count>();
  epArray<T, Count>(nullptr_t);
  epArray<T, Count>(std::initializer_list<T> list);
  epArray<T, Count>(const epArray<T, Count> &val);
  epArray<T, Count>(epArray<T, Count> &&rval);
  template <typename U> epArray<T, Count>(U *ptr, size_t length);
  template <typename U> epArray<T, Count>(epSlice<U> slice);
  ~epArray<T, Count>();

  void reserve(size_t count);

  // assignment
  epArray<T, Count>& operator =(epArray<T, Count> &&rval);
  template <typename U> epArray<T, Count>& operator =(epSlice<U> rh);

  // manipulation
  void clear();
  template <typename... Things> epArray<T, Count>& concat(const Things&... things);

  T& front() const;
  T popFront();

  T& pushBack();
  template <typename U> epArray<T, Count>& pushBack(U &&item);

  void remove(size_t i);
  void remove(const T *pItem);
  void removeFirst(const T &item) { remove(this->findFirst(item)); }

  void removeSwapLast(size_t i);
  void removeSwapLast(const T *pItem);
  void removeFirstSwapLast(const T &item) { removeSwapLast(this->findFirst(item)); }

  epSlice<T> getBuffer() const;

protected:
  template<size_t Len, bool = true>
  struct Buffer
  {
    char buffer[sizeof(T) * Len];
    bool hasAllocation(T *p) const { return p != (T*)buffer && p != nullptr; }
    T* ptr() const { return (T*)buffer; }
  };
  template<bool dummy> struct Buffer<0, dummy> // SORRY! >_< C++ still sucks! We must partial-specialise here because reasons.
  {
    bool hasAllocation(T *p) const { return p != nullptr; }
    T* ptr() const { return nullptr; }
  };
  Buffer<Count> buffer;

  struct Header
  {
    union
    {
      char header[64];
      size_t numAllocated;
    };
  };
  static size_t numToAlloc(size_t i);
  bool hasAllocation() const { return buffer.hasAllocation(this->ptr); }
  Header* getHeader() const { return ((Header*)this->ptr) - 1; }
};


// epSharedSlice is a reference counted slice, used to retain ownership of some memory, but will not duplicate when copies are made
// useful for long-living data that doesn't consume a fixed amount of their containing struct
// also useful for sharing between systems, passing between threads, etc.
// slices of epSharedSlice's increment the RC, so that slices can outlive the original owner, but without performing additional allocations and copies
template <typename T>
struct epSharedSlice : public epSlice<T>
{
  epRC *rc;

  // constructors
  epSharedSlice<T>();
  epSharedSlice<T>(nullptr_t);
  epSharedSlice<T>(std::initializer_list<typename epSharedSlice<T>::ET> list);
  epSharedSlice<T>(epSharedSlice<T> &&rval);
  epSharedSlice<T>(const epSharedSlice<T> &rcslice);
  template <typename U> epSharedSlice<T>(U *ptr, size_t length);
  template <typename U> epSharedSlice<T>(epSlice<U> slice);
  ~epSharedSlice<T>();

  size_t refcount() const { return rc ? rc->refCount : 0; }

  // static constructors (make proper constructors?)
  template<typename... Things> static epSharedSlice<T> concat(const Things&... things);
  static epSharedSlice<T> alloc(size_t elements);

  // assignment
  epSharedSlice<T>& operator =(const epSharedSlice<T> &rh);
  epSharedSlice<T>& operator =(epSharedSlice<T> &&rval);
  template <typename U> epSharedSlice<T>& operator =(epSlice<U> rh);

  // contents
  epSharedSlice<T> slice(size_t first, size_t last) const;

protected:
  epSharedSlice<T>(T *ptr, size_t length, epRC *rc);
  static size_t numToAlloc(size_t i);
  template <typename U> static epSlice<T> alloc(U *ptr, size_t length);
  template <typename U> void init(U *ptr, size_t length);
};

// unit tests
udResult epSlice_Test();

#endif // _EP_SLICE
