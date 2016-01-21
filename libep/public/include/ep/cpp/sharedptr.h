#pragma once
#if !defined(_EPSHAREDPTR_HPP)
#define _EPSHAREDPTR_HPP

#include <utility>

#include "ep/cpp/platform.h"

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


// **HAX** this allows us to delete a RefCounted!
// Some classes that derive off RefCounted have private/protected destructors. For ~UniquePtr() to be able to
// destroy a reference to one of these it has to cast to a RefCount* first to be able to
// call delete. Thee two specialisations of Destroy facilitate that.

namespace internal {

  template<typename T, bool isref>
  struct Destroy;
  template<class T>
  struct Destroy<T, false> {
    epforceinline static void destroy(T *ptr);
  };
  template<class T>
  struct Destroy<T, true> {
    epforceinline static void destroy(T *ptr);
  };

} // namespace internal

class Safe
{
protected:
  virtual ~Safe();
};

template<typename U> class SafePtr;

// shared pointers are ref counted
template<class T>
class SharedPtr
{
public:
  using Type = T;

  // create a new instance of T
  template<typename... Args>
  static SharedPtr<T> create(Args... args)
  {
    return SharedPtr<T>(new T(args...));
  }

  SharedPtr() {}
  SharedPtr(nullptr_t) {}

  // constructors
  SharedPtr(const SharedPtr<T> &ptr)
    : pInstance(acquire(ptr.pInstance)) {}
  SharedPtr(SharedPtr<T> &&ptr)
    : pInstance(ptr.pInstance)
  {
    if (this != &ptr)
      ptr.pInstance = nullptr;
  }

  // the U allows us to accept const
  template <class U>
  SharedPtr(const SharedPtr<U> &ptr)
    : pInstance(acquire(ptr.pInstance)) {}
  template <class U>
  SharedPtr(SharedPtr<U> &&ptr)
    : pInstance(ptr.pInstance)
  {
    if (this != (void*)&ptr)
      ptr.pInstance = nullptr;
  }

  template <class U> // the U allows us to accept const
  SharedPtr(UniquePtr<U> &ptr);
  SharedPtr(UniquePtr<T> &&ptr);

  template <class U>
  SharedPtr(const SafePtr<U> &ptr)
    : SharedPtr<T>(ptr.ptr())
  {
  }

  explicit SharedPtr(T *p)
    : pInstance(acquire(p)) {}

  ~SharedPtr() { release(pInstance); }

  SharedPtr& operator=(SharedPtr<T> &&ptr)
  {
    if (pInstance != ptr.pInstance)
    {
      RefCounted *pOld = pInstance;
      pInstance = ptr.pInstance;
      ptr.pInstance = nullptr;
      release(pOld);
    }
    return *this;
  }
  SharedPtr& operator=(const SharedPtr<T> &ptr)
  {
    if (pInstance != ptr.pInstance)
    {
      RefCounted *pOld = pInstance;
      pInstance = acquire(ptr.pInstance);
      release(pOld);
    }
    return *this;
  }
  template <class U> // the U allows us to accept const
  SharedPtr& operator=(const SharedPtr<U> &ptr)
  {
    if (pInstance != ptr.pInstance)
    {
      RefCounted *pOld = pInstance;
      pInstance = acquire(ptr.pInstance);
      release(pOld);
    }
    return *this;
  }
  SharedPtr& operator=(UniquePtr<T> &&ptr);
  template <class U> // the U allows us to accept const
  SharedPtr& operator=(UniquePtr<U> &ptr);
  SharedPtr& operator=(nullptr_t)
  {
    RefCounted *pOld = pInstance;
    pInstance = nullptr;
    release(pOld);
    return *this;
  }

  template <class U>
  SharedPtr& operator=(SafePtr<U> &ptr)
  {
    RefCounted *pOld = pInstance;
    pInstance = acquire(ptr.ptr());
    release(pOld);
    return *this;
  }

  void reset()
  {
    RefCounted *pOld = pInstance;
    pInstance = nullptr;
    release(pOld);
  }

  size_t count() const;

  epforceinline bool unique() const { return count() == 1; }

  epforceinline explicit operator bool() const { return count() > 0; }

  T& operator*() const { return *(T*)pInstance; }
  T* operator->() const { return (T*)pInstance; }
  T* ptr() const { return (T*)pInstance; }

private:
  template<typename U> friend class UniquePtr;
  template<typename U> friend class SharedPtr;
  template<typename U> friend class SafePtr;

  static RefCounted* acquire(RefCounted *pI);
  static void release(RefCounted *pI);

  RefCounted *pInstance = nullptr;
};

//------------------------------------------------------------------------------------------

// unique pointers nullify the source pointer on assignment
template<typename T>
class UniquePtr
{
public:
  UniquePtr() {}
  UniquePtr(nullptr_t) {}

  explicit UniquePtr(T *p) : pInstance(p) {}

  UniquePtr(const UniquePtr<T> &ptr) = delete;
  UniquePtr &operator=(const UniquePtr<T> &ptr) = delete;

  template<typename U>
  UniquePtr(UniquePtr<U> &ptr)
    : pInstance(ptr.pInstance)
  {
    ptr.pInstance = nullptr;
  }
  UniquePtr(UniquePtr<T> &&ptr)
    : pInstance(ptr.pInstance)
  {
    ptr.pInstance = nullptr;
  }
  template<typename U>
  UniquePtr(UniquePtr<U> &&ptr)
    : pInstance(ptr.pInstance)
  {
    ptr.pInstance = nullptr;
  }

  ~UniquePtr()
  {
    internal::Destroy<T, std::is_base_of<RefCounted, T>::value>::destroy(pInstance);
  }

  template<typename U>
  UniquePtr &operator=(UniquePtr<U> &ptr)
  {
    reset();
    pInstance = ptr.pInstance;
    ptr.pInstance = nullptr;
    return *this;
  }
  UniquePtr &operator=(UniquePtr<T> &&ptr)
  {
    reset();
    pInstance = ptr.pInstance;
    ptr.pInstance = nullptr;
    return *this;
  }
  template<typename U>
  UniquePtr &operator=(UniquePtr<U> &&ptr)
  {
    reset();
    pInstance = ptr.pInstance;
    ptr.pInstance = nullptr;
    return *this;
  }

  void reset()
  {
    this->~UniquePtr();
    pInstance = nullptr;
  }

  void release()
  {
    pInstance = nullptr;
  }

  // reference counter operations:
  explicit operator bool() const { return pInstance != nullptr; }

  // underlying pointer operations:
  T& operator*() const { return *pInstance; }
  T* operator->() const { return pInstance; }
  T* ptr() const { return pInstance; }

private:
  template<typename U> friend class SharedPtr;
  template<typename U> friend class UniquePtr;
  template<typename U, bool isref> friend struct Destroy;

  T * eprestrict pInstance = nullptr;
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

  ~SynchronisedPtr()
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

  void reset()
  {
    destroy();
  }

  void release()
  {
    pInstance = nullptr;
  }

  // reference counter operations :
  explicit operator bool() const { return pInstance != nullptr; }

  // underlying pointer operations :
  T& operator*() const { return *pInstance; }
  T* operator->() const { return pInstance; }
  T* ptr() const { return pInstance; }

private:
  template<typename U> friend class SynchronisedPtr;

  void destroy();

  T *pInstance = nullptr;
  Kernel *pKernel = nullptr;
};


// ref counting base class
class RefCounted : public Safe
{
  mutable size_t rc = 0;

public:
  size_t RefCount() { return rc; }
  size_t IncRef() { return ++rc; }
  size_t DecRef()
  {
    if (rc == 1)
      delete this;
    return --rc;
  }

  template<typename T>
  friend class SharedPtr;
  template<typename T, bool isrc>
  friend struct internal::Destroy;
};


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
inline SharedPtr<T>::SharedPtr(UniquePtr<U> &ptr)
  : pInstance(ptr.pInstance)
{
  if (pInstance)
  {
    pInstance->rc = 1;
    ptr.pInstance = nullptr;
  }
}

template <class T>
inline SharedPtr<T>& SharedPtr<T>::operator=(UniquePtr<T> &&ptr)
{
  RefCounted *pOld = pInstance;
  pInstance = ptr.pInstance;
  if (pInstance)
  {
    pInstance->rc = 1;
    ptr.pInstance = nullptr;
  }
  release(pOld);
  return *this;
}

template <class T>
template <class U>
inline SharedPtr<T>& SharedPtr<T>::operator=(UniquePtr<U> &ptr)
{
  RefCounted *pOld = pInstance;
  pInstance = ptr.pInstance;
  if (pInstance)
  {
    pInstance->rc = 1;
    ptr.pInstance = nullptr;
  }
  release(pOld);
  return *this;
}

template<class T>
epforceinline size_t SharedPtr<T>::count() const
{
  return pInstance ? pInstance->rc : 0;
}

template<class T>
epforceinline RefCounted* SharedPtr<T>::acquire(RefCounted *pI)
{
  if (pI)
    ++pI->rc;
  return pI;
}
template<class T>
inline void SharedPtr<T>::release(RefCounted *pI)
{
  if (!pI)
    return;
  if (pI->rc == 1)
    delete pI;
  else
    --pI->rc;
}

namespace internal {

  template<class T>
  epforceinline void Destroy<T, false>::destroy(T *ptr)
  {
    delete ptr;
  }
  template<class T>
  epforceinline void Destroy<T, true>::destroy(T *ptr)
  {
    RefCounted *rc = ptr;
    delete rc;
  }

} // namespace internal

// cast functions
template<class T, class U>
UniquePtr<T> unique_pointer_cast(const UniquePtr<U> &ptr);
template<class T, class U>
SharedPtr<T> shared_pointer_cast(const SharedPtr<U> &ptr);

// comparaison operators
template<class T, class U> inline bool operator==(const SharedPtr<T> &l, const SharedPtr<U> &r) { return l.ptr() == r.ptr(); }
template<class T, class U> inline bool operator!=(const SharedPtr<T> &l, const SharedPtr<U> &r) { return l.ptr() != r.ptr(); }
template<class T, class U> inline bool operator<=(const SharedPtr<T> &l, const SharedPtr<U> &r) { return l.ptr() <= r.ptr(); }
template<class T, class U> inline bool operator<(const SharedPtr<T> &l, const SharedPtr<U> &r) { return l.ptr() < r.ptr(); }
template<class T, class U> inline bool operator>=(const SharedPtr<T> &l, const SharedPtr<U> &r) { return l.ptr() >= r.ptr(); }
template<class T, class U> inline bool operator>(const SharedPtr<T> &l, const SharedPtr<U> &r) { return l.ptr() > r.ptr(); }
template<class T> inline bool operator==(const SharedPtr<T> &l, nullptr_t) { return l.ptr() == nullptr; }
template<class T> inline bool operator!=(const SharedPtr<T> &l, nullptr_t) { return l.ptr() != nullptr; }
template<class T> inline bool operator==(nullptr_t, const SharedPtr<T> &r) { return nullptr == r.ptr(); }
template<class T> inline bool operator!=(nullptr_t, const SharedPtr<T> &r) { return nullptr != r.ptr(); }

template<class T, class U> inline bool operator==(const UniquePtr<T> &l, const UniquePtr<U> &r) { return l.ptr() == r.ptr(); }
template<class T, class U> inline bool operator!=(const UniquePtr<T> &l, const UniquePtr<U> &r) { return l.ptr() != r.ptr(); }
template<class T, class U> inline bool operator<=(const UniquePtr<T> &l, const UniquePtr<U> &r) { return l.ptr() <= r.ptr(); }
template<class T, class U> inline bool operator<(const UniquePtr<T> &l, const UniquePtr<U> &r) { return l.ptr() < r.ptr(); }
template<class T, class U> inline bool operator>=(const UniquePtr<T> &l, const UniquePtr<U> &r) { return l.ptr() >= r.ptr(); }
template<class T, class U> inline bool operator>(const UniquePtr<T> &l, const UniquePtr<U> &r) { return l.ptr() > r.ptr(); }
template<class T> inline bool operator==(const UniquePtr<T> &l, nullptr_t) { return l.ptr() == nullptr; }
template<class T> inline bool operator!=(const UniquePtr<T> &l, nullptr_t) { return l.ptr() != nullptr; }
template<class T> inline bool operator==(nullptr_t, const UniquePtr<T> &r) { return nullptr == r.ptr(); }
template<class T> inline bool operator!=(nullptr_t, const UniquePtr<T> &r) { return nullptr != r.ptr(); }

} // namespace ep

#include "ep/cpp/safeptr.h"
#include "ep/cpp/internal/sharedptr_inl.h"

#endif // _EPSHAREDPTR_HPP
