#if !defined(_UD_SLICE)
#define _UD_SLICE

#include "udPlatform.h"
#include "udResult.h"

#include <initializer_list>

struct udRC
{
  size_t refCount;
  size_t allocatedCount;
};


// slices are bounded arrays, unlike C's conventional unbounded pointers (typically, with separate length stored in parallel)
// no attempt is made to create a one-size-fits-all implementation, as it is recognised that usages offer distinct advantages/disadvantages
// slise is the basis of the suite however, and everything is based on udSlice. derived types address specifics in usage and/or ownership patterns

// declare an iterator so it works with standard range pased functions (foreach!)
template<typename T>
class udIterator
{
  // TODO: this could be made safer by storing a ref to the parent slice, and keeping an offset
  T *pI;

public:
  udIterator(T *pI) : pI(pI) {}
  bool operator!=(udIterator<T> rh) const { return pI != rh.pI; } // compare
  udIterator<T> operator++() { ++pI; return *this; }              // increment
  T& operator*() const { return *pI; }                            // value
};

// udSlice does not retain ownership of it's memory, it is used for temporary ownership; working locals, function args, etc
template<typename T>
struct udSlice
{
private:
  template<typename ET> struct ElementType { typedef ET Ty; };
  template<> struct ElementType<void> { typedef uint8_t Ty; };
  template<> struct ElementType<const void> { typedef const uint8_t Ty; };
public:
  typedef typename ElementType<T>::Ty ET;

  size_t length;
  T *ptr;

  // constructors
  udSlice<T>();
  udSlice<T>(nullptr_t);
  udSlice<T>(std::initializer_list<ET> list);
  udSlice<T>(T* ptr, size_t length);
  template<typename U> udSlice<T>(udSlice<U> rh);

  // assignment
  template<typename U> udSlice<T>& operator =(udSlice<U> rh);

  // contents
  ET& operator[](ptrdiff_t i) const;

  udSlice<T> slice(ptrdiff_t first, ptrdiff_t last) const;

  bool empty() const;
  explicit operator bool() const { return length != 0; }

  // comparison
  bool operator ==(udSlice<const T> rh) const;
  bool operator !=(udSlice<const T> rh) const;

  template<typename U> bool eq(udSlice<U> rh) const;
  template<typename U> ptrdiff_t cmp(udSlice<U> rh) const;

  template<typename U> bool beginsWith(udSlice<U> rh) const;
  template<typename U> bool endsWith(udSlice<U> rh) const;

  // iterators
  udIterator<T> begin() const;
  udIterator<T> end() const;

  // useful functions
  ET& front() const;
  ET& back() const;
  ET& popFront();
  ET& popBack();
  udSlice<T> get(ptrdiff_t n) const;
  udSlice<T> pop(ptrdiff_t n);
  udSlice<T> strip(ptrdiff_t n) const;

  bool exists(const ET &c, size_t *pIndex = nullptr) const;
  size_t findFirst(const ET &c) const;
  size_t findLast(const ET &c) const;
  template<typename U> size_t findFirst(udSlice<U> s) const;
  template<typename U> size_t findLast(udSlice<U> s) const;

  udSlice<T> getLeftAtFirst(const ET &c, bool bInclusive = false) const;
  udSlice<T> getLeftAtLast(const ET &c, bool bInclusive = false) const;
  udSlice<T> getRightAtFirst(const ET &c, bool bInclusive = true) const;
  udSlice<T> getRightAtLast(const ET &c, bool bInclusive = true) const;

  template<typename U> udSlice<T> getLeftAtFirst(udSlice<U> s, bool bInclusive = false) const;
  template<typename U> udSlice<T> getLeftAtLast(udSlice<U> s, bool bInclusive = false) const;
  template<typename U> udSlice<T> getRightAtFirst(udSlice<U> s, bool bInclusive = true) const;
  template<typename U> udSlice<T> getRightAtLast(udSlice<U> s, bool bInclusive = true) const;

  ptrdiff_t indexOfElement(const T *c) const;

  typedef bool(*Predicate)(const ET &e);
  T* search(Predicate) const;

  template<bool skipEmptyTokens = false>
  udSlice<T> popToken(udSlice<T> delimiters);

  template<bool skipEmptyTokens = false>
  udSlice<udSlice<T>> tokenise(udSlice<udSlice<T>> tokens, udSlice<T> delimiters);

  template<typename U>
  void copyTo(udSlice<U> dest) const;
};

// udFixedSlice introduces static-sized and/or stack-based ownership. this is useful anywhere that fixed-length arrays are appropriate
// udFixedSlice will fail-over to an allocated buffer if the contents exceed the fixed size
template <typename T, size_t Count = 0>
struct udFixedSlice : public udSlice<T>
{
  // constructors
  udFixedSlice<T, Count>();
  udFixedSlice<T, Count>(nullptr_t);
  udFixedSlice<T, Count>(std::initializer_list<T> list);
  udFixedSlice<T, Count>(const udFixedSlice<T, Count> &val);
  udFixedSlice<T, Count>(udFixedSlice<T, Count> &&rval);
  template <typename U> udFixedSlice<T, Count>(U *ptr, size_t length);
  template <typename U> udFixedSlice<T, Count>(udSlice<U> slice);
  ~udFixedSlice<T, Count>();

  void reserve(size_t count);

  // assignment
  udFixedSlice<T, Count>& operator =(udFixedSlice<T, Count> &&rval);
  template <typename U> udFixedSlice<T, Count>& operator =(udSlice<U> rh);

  // manipulation
  void clear();
  template <typename... Things> udFixedSlice<T, Count>& concat(const Things&... things);

  T& front() const;
  T popFront();

  T& pushBack();
  template <typename U> udFixedSlice<T, Count>& pushBack(U &&item);

  void remove(size_t i);
  void remove(const T *pItem);
  void removeFirst(const T &item) { remove(this->findFirst(item)); }

  void removeSwapLast(size_t i);
  void removeSwapLast(const T *pItem);
  void removeFirstSwapLast(const T &item) { removeSwapLast(this->findFirst(item)); }

  udSlice<T> getBuffer() const;

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


// udSharedSlice is a reference counted slice, used to retain ownership of some memory, but will not duplicate when copies are made
// useful for long-living data that doesn't consume a fixed amount of their containing struct
// also useful for sharing between systems, passing between threads, etc.
// slices of udSharedSlice's increment the RC, so that slices can outlive the original owner, but without performing additional allocations and copies
template <typename T>
struct udSharedSlice : public udSlice<T>
{
  udRC *rc;

  // constructors
  udSharedSlice<T>();
  udSharedSlice<T>(nullptr_t);
  udSharedSlice<T>(std::initializer_list<ET> list);
  udSharedSlice<T>(udSharedSlice<T> &&rval);
  udSharedSlice<T>(const udSharedSlice<T> &rcslice);
  template <typename U> udSharedSlice<T>(U *ptr, size_t length);
  template <typename U> udSharedSlice<T>(udSlice<U> slice);
  ~udSharedSlice<T>();

  size_t refcount() const { return rc ? rc->refCount : 0; }

  // static constructors (make proper constructors?)
  template<typename... Things> static udSharedSlice<T> concat(const Things&... things);
  static udSharedSlice<T> alloc(size_t elements);

  // assignment
  udSharedSlice<T>& operator =(const udSharedSlice<T> &rh);
  udSharedSlice<T>& operator =(udSharedSlice<T> &&rval);
  template <typename U> udSharedSlice<T>& operator =(udSlice<U> rh);

  // contents
  udSharedSlice<T> slice(size_t first, size_t last) const;

protected:
  udSharedSlice<T>(T *ptr, size_t length, udRC *rc);
  static size_t numToAlloc(size_t i);
  template <typename U> static udSlice<T> alloc(U *ptr, size_t length);
  template <typename U> void init(U *ptr, size_t length);
};

// unit tests
udResult udSlice_Test();


#include "ep/epslice.inl"

#endif // _UD_SLICE
