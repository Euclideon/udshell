#pragma once
#if !defined(_EPSHAREDPTR_HPP)
#define _EPSHAREDPTR_HPP

#include <type_traits>

#include "ep/cpp/platform.h"

#if defined(EP_COMPILER_VISUALC)
// TODO: REMOVE THIS!!!
#pragma warning(disable: 4714)
#endif

#define SHARED_CLASS(Name) \
  class Name; \
  typedef ::ep::SharedPtr<Name> Name##Ref;

#define SHARED_STRUCT(Name) \
  struct Name; \
  typedef ::ep::SharedPtr<Name> Name##Ref;

namespace ep {

class Kernel;
class RefCounted;
template<class T>
class UniquePtr;


// weak pointer is just an alias for obvious annotation in code, may be fleshed out for debug at some point...
template<typename T, typename std::enable_if<std::is_base_of<RefCounted, T>::value>::type* = nullptr>
using WeakPtr = T*;


// shared pointers are ref counted
template<class T>
class SharedPtr
{
public:
  // create a new instance of T
  template<typename... Args>
  static SharedPtr<T> create(Args... args)
  {
    return SharedPtr<T>(new T(args...));
  }

  // constructors
  SharedPtr(const SharedPtr<T> &ptr) : pInstance(ptr.pInstance) { acquire(); }
  template <class U> // the U allows us to accept const
  SharedPtr(const SharedPtr<U> &ptr) : pInstance(ptr.pInstance) { acquire(); }
  SharedPtr(SharedPtr<T> &&ptr)
    : pInstance(ptr.pInstance)
  {
    if (this != &ptr)
      ptr.pInstance = nullptr;
  }
  template <class U> // the U allows us to accept const
  SharedPtr(const UniquePtr<U> &ptr);
  SharedPtr(UniquePtr<T> &&ptr);

  SharedPtr() {}
  SharedPtr(nullptr_t) {}

  explicit SharedPtr(T *p) : pInstance(p) { acquire(); }

  inline ~SharedPtr() { release(); }

  SharedPtr& operator=(SharedPtr<T> &&ptr)
  {
    if (pInstance != ptr.pInstance)
    {
      release();
      pInstance = ptr.pInstance;
      ptr.pInstance = nullptr;
    }
    return *this;
  }
  SharedPtr& operator=(const SharedPtr<T> &ptr)
  {
    if (pInstance != ptr.pInstance)
    {
      release();
      pInstance = ptr.pInstance;
      acquire();
    }
    return *this;
  }
  template <class U> // the U allows us to accept const
  SharedPtr& operator=(const SharedPtr<U> &ptr)
  {
    if (pInstance != ptr.pInstance)
    {
      release();
      pInstance = ptr.pInstance;
      acquire();
    }
    return *this;
  }
  SharedPtr& operator=(UniquePtr<T> &&ptr);
  template <class U> // the U allows us to accept const
  SharedPtr& operator=(const UniquePtr<U> &ptr);
  SharedPtr& operator=(nullptr_t)
  {
    release();
    return *this;
  }

  inline void reset()
  {
    release();
  }

  inline size_t count() const;

  epforceinline bool unique() const { return count() == 1; }

  epforceinline explicit operator bool() const { return count() > 0; }

  epforceinline T& operator*() const { return *(T*)pInstance; }
  epforceinline T* operator->() const { return (T*)pInstance; }
  epforceinline T* ptr() const { return (T*)pInstance; }

private:
  template<class U> friend class SharedPtr;

  inline void acquire();
  inline void release();

  RefCounted *pInstance = nullptr;
};

//------------------------------------------------------------------------------------------

// **HAX** this crap allows us to delete a RefCounted!
namespace internal {

template<typename T, bool isref>
struct Destroy;
template<class T>
struct Destroy<T, false> {
  epforceinline static void destroy(T *ptr)
  {
    delete ptr;
  }
};
template<class T>
struct Destroy<T, true> {
  epforceinline static void destroy(T *ptr)
  {
    RefCounted *rc = ptr;
    delete rc;
  }
};

} // namespace internal

// unique pointers nullify the source pointer on assignment
template<class T>
class UniquePtr
{
public:
  UniquePtr() {}
  explicit UniquePtr(T *p) : pInstance(p) {}
  template <class U>
  UniquePtr(const UniquePtr<U> &ptr)
    : pInstance(ptr.pInstance)
  {
    ptr.pInstance = nullptr;
  }
  UniquePtr(const UniquePtr &ptr)
    : pInstance(ptr.pInstance)
  {
    ptr.pInstance = nullptr;
  }
  ~UniquePtr()
  {
    internal::Destroy<T, std::is_base_of<RefCounted, T>::value>::destroy(pInstance);
  }

  UniquePtr &operator=(const UniquePtr &ptr)
  {
    reset();
    pInstance = ptr.pInstance;
    ptr.pInstance = nullptr;
    return *this;
  }

  inline void reset()
  {
    this->~UniquePtr();
    pInstance = nullptr;
  }

  inline void release()
  {
    pInstance = nullptr;
  }

  // reference counter operations :
  epforceinline explicit operator bool() const { return pInstance != nullptr; }

  // underlying pointer operations :
  epforceinline T& operator*() const { return *pInstance; }
  epforceinline T* operator->() const { return pInstance; }
  epforceinline T* ptr() const { return pInstance; }

private:
  template<typename U> friend class SharedPtr;
  template<typename U> friend class UniquePtr;
  template<typename U, bool isref> friend struct Destroy;

  mutable T *pInstance = nullptr;
};

//------------------------------------------------------------------------------------------

// synchronised pointers that can be passed between threads, destruction is deferred to main thread
template<class T>
class SynchronisedPtr
{
public:
  SynchronisedPtr() {}
  template <class U>
  explicit SynchronisedPtr(const SharedPtr<U> &p, Kernel *pKernel)
    : pInstance(p.ptr())
    , pKernel(pKernel)
  {
    p.acquire();
  }
  template <class U>
  SynchronisedPtr(const SynchronisedPtr<U> &ptr)
    : pInstance(ptr.pInstance)
    , pKernel(ptr.pKernel)
  {
    const_cast<SynchronisedPtr<U>&>(ptr).pInstance = nullptr;
    const_cast<SynchronisedPtr<U>&>(ptr).pKernel = nullptr;
  }
  SynchronisedPtr(const SynchronisedPtr &ptr)
    : pInstance(ptr.pInstance)
    , pKernel(ptr.pKernel)
  {
    const_cast<SynchronisedPtr&>(ptr).pInstance = nullptr;
    const_cast<SynchronisedPtr&>(ptr).pKernel = nullptr;
  }
  inline ~SynchronisedPtr()
  {
    destroy();
  }

  SynchronisedPtr &operator=(const SynchronisedPtr &ptr)
  {
    destroy();
    pInstance = ptr.pInstance;
    pKernel = ptr.pKernel;
    const_cast<SynchronisedPtr&>(ptr).pInstance = nullptr;
    const_cast<SynchronisedPtr&>(ptr).pKernel = nullptr;
    return *this;
  }

  inline void reset()
  {
    destroy();
  }

  inline void release()
  {
    pInstance = nullptr;
  }

  // reference counter operations :
  inline explicit operator bool() const { return pInstance != nullptr; }

  // underlying pointer operations :
  inline T& operator*() const { return *pInstance; }
  inline T* operator->() const { return pInstance; }
  inline T* ptr() const { return pInstance; }

private:
  template<typename U> friend class SynchronisedPtr;

  void destroy();

  T *pInstance = nullptr;
  Kernel *pKernel = nullptr;
};


// ref counting base class
class RefCounted
{
  mutable size_t rc = 0;

protected:
  virtual ~RefCounted() = 0;
public:
  size_t RefCount() { return rc; }
  size_t IncRef() { return ++rc; }
  size_t DecRef()
  {
    if (--rc == 0)
      delete this;
    return rc;
  }

  template<typename T>
  friend class SharedPtr;
  template<typename T, bool isrc>
  friend struct internal::Destroy;
};
inline RefCounted::~RefCounted() {}


// SharedPtr constructors and assignments
template <class T>
inline SharedPtr<T>::SharedPtr(UniquePtr<T> &&ptr)
  : pInstance(ptr.pInstance)
{
  pInstance->rc = 1;
  ptr.pInstance = nullptr;
}
template <class T>
template <class U>
inline SharedPtr<T>::SharedPtr(const UniquePtr<U> &ptr)
  : pInstance(ptr.pInstance)
{
  pInstance->rc = 1;
  ptr.pInstance = nullptr;
}
template <class T>
inline SharedPtr<T>& SharedPtr<T>::operator=(UniquePtr<T> &&ptr)
{
  release();
  pInstance = ptr.pInstance;
  pInstance->rc = 1;
  ptr.pInstance = nullptr;
  return *this;
}
template <class T>
template <class U>
inline SharedPtr<T>& SharedPtr<T>::operator=(const UniquePtr<U> &ptr)
{
  release();
  pInstance = ptr.pInstance;
  pInstance->rc = 1;
  ptr.pInstance = nullptr;
  return *this;
}

template<class T>
epforceinline size_t SharedPtr<T>::count() const
{
  return pInstance ? pInstance->rc : 0;
}

template<class T>
epforceinline void SharedPtr<T>::acquire()
{
  if (pInstance)
  {
    ++pInstance->rc;
  }
}
template<class T>
inline void SharedPtr<T>::release()
{
  if (pInstance)
  {
    if (--pInstance->rc == 0)
      delete pInstance;
    pInstance = nullptr;
  }
}

} // namespace ep


// cast functions
template<class T, class U>
inline SharedPtr<T> shared_pointer_cast(const SharedPtr<U> &ptr)
{
  return SharedPtr<T>((T*)ptr.ptr());
}

// comparaison operators
template<class T, class U> inline bool operator==(const SharedPtr<T> &l, const SharedPtr<U> &r) { return l.ptr() == r.ptr(); }
template<class T, class U> inline bool operator!=(const SharedPtr<T> &l, const SharedPtr<U> &r) { return l.ptr() != r.ptr(); }
template<class T, class U> inline bool operator<=(const SharedPtr<T> &l, const SharedPtr<U> &r) { return l.ptr() <= r.ptr(); }
template<class T, class U> inline bool operator<(const SharedPtr<T> &l, const SharedPtr<U> &r) { return l.ptr() < r.ptr(); }
template<class T, class U> inline bool operator>=(const SharedPtr<T> &l, const SharedPtr<U> &r) { return l.ptr() >= r.ptr(); }
template<class T, class U> inline bool operator>(const SharedPtr<T> &l, const SharedPtr<U> &r) { return l.ptr() > r.ptr(); }
template<class T, class U> inline bool operator==(const SharedPtr<T> &l, nullptr_t) { return l.ptr() == nullptr; }
template<class T, class U> inline bool operator!=(const SharedPtr<T> &l, nullptr_t) { return l.ptr() != nullptr; }
template<class T, class U> inline bool operator==(nullptr_t, const SharedPtr<U> &r) { return nullptr == r.ptr(); }
template<class T, class U> inline bool operator!=(nullptr_t, const SharedPtr<U> &r) { return nullptr != r.ptr(); }

template<class T, class U> inline bool operator==(const UniquePtr<T> &l, const UniquePtr<U> &r) { return l.ptr() == r.ptr(); }
template<class T, class U> inline bool operator!=(const UniquePtr<T> &l, const UniquePtr<U> &r) { return l.ptr() != r.ptr(); }
template<class T, class U> inline bool operator<=(const UniquePtr<T> &l, const UniquePtr<U> &r) { return l.ptr() <= r.ptr(); }
template<class T, class U> inline bool operator<(const UniquePtr<T> &l, const UniquePtr<U> &r) { return l.ptr() < r.ptr(); }
template<class T, class U> inline bool operator>=(const UniquePtr<T> &l, const UniquePtr<U> &r) { return l.ptr() >= r.ptr(); }
template<class T, class U> inline bool operator>(const UniquePtr<T> &l, const UniquePtr<U> &r) { return l.ptr() > r.ptr(); }
template<class T, class U> inline bool operator==(const UniquePtr<T> &l, nullptr_t) { return l.ptr() == nullptr; }
template<class T, class U> inline bool operator!=(const UniquePtr<T> &l, nullptr_t) { return l.ptr() != nullptr; }
template<class T, class U> inline bool operator==(nullptr_t, const UniquePtr<U> &r) { return nullptr == r.ptr(); }
template<class T, class U> inline bool operator!=(nullptr_t, const UniquePtr<U> &r) { return nullptr != r.ptr(); }


template<typename T>
ptrdiff_t epStringify(Slice<char> buffer, String format, SharedPtr<T> spT, const epVarArg *pArgs)
{
  return epStringifyTemplate(buffer, format, spT.ptr(), pArgs);
}

#endif // _EPSHAREDPTR_HPP
