#pragma once
#if !defined(_EPSHAREDPTR_HPP)
#define _EPSHAREDPTR_HPP

#include <type_traits>

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
template<class T>
class WeakPtr;
namespace internal {
  template<class T>
  class WeakProxy;
}


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

  void* GetWeakPtr(void *pAlloc);
  void NullifyWeakPtr(void *pAlloc);

} // namespace internal


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

  SharedPtr() {}
  SharedPtr(nullptr_t) {}

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

  template<typename U>
  SharedPtr(const WeakPtr<U> &ptr);

  explicit SharedPtr(T *p) : pInstance(p) { acquire(); }

  ~SharedPtr() { release(); }

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

  void reset()
  {
    release();
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
  template<typename U> friend class WeakPtr;

  void acquire();
  void release();

  RefCounted *pInstance = nullptr;
};

//------------------------------------------------------------------------------------------


template<typename T>
class WeakPtr
{
public:
  WeakPtr(const WeakPtr<T> &ptr) : spProxy(ptr.spProxy) {}
  template <class U> // the U allows us to accept const
  WeakPtr(const WeakPtr<U> &ptr) : spProxy(ptr.spProxy) {}
  WeakPtr(WeakPtr<T> &&ptr) : spProxy(std::move(ptr.spProxy)) {}

  WeakPtr() {}
  WeakPtr(nullptr_t) {}

  explicit WeakPtr(T *pInstance) : spProxy((internal::WeakProxy<T>*)internal::GetWeakPtr(pInstance)) {}
  explicit WeakPtr(const SharedPtr<T> &spInstance) : spProxy((internal::WeakProxy<T>*)internal::GetWeakPtr(spInstance.ptr())) {}
  explicit WeakPtr(const UniquePtr<T> &upInstance) : spProxy((internal::WeakProxy<T>*)internal::GetWeakPtr(upInstance.ptr())) {}

  WeakPtr& operator=(const WeakPtr<T> &ptr)
  {
    spProxy = ptr.spProxy;
    return *this;
  }
  template<typename U>
  WeakPtr& operator=(const WeakPtr<U> &ptr)
  {
    spProxy = ptr.spProxy;
    return *this;
  }

  epforceinline explicit operator bool() const { return spProxy.count() > 0 && spProxy->pInstance; }

  epforceinline T& operator*() const { return *(T*)spProxy->pInstance; }
  epforceinline T* operator->() const { return (T*)spProxy->pInstance; }
  epforceinline T* ptr() const { return spProxy ? (T*)spProxy->pInstance : (T*)nullptr; }

  static_assert(std::is_base_of<RefCounted, T>::value, "T must inherit from RefCounted");

private:
  SharedPtr<internal::WeakProxy<T>> spProxy;
};


// unique pointers nullify the source pointer on assignment
template<class T>
class UniquePtr
{
public:
  UniquePtr() {}
  UniquePtr(nullptr_t) {}

  explicit UniquePtr(T *p) : pInstance(p) {}

  UniquePtr(const UniquePtr<T> &ptr)
    : pInstance(ptr.pInstance)
  {
    ptr.pInstance = nullptr;
  }
  template<class U>
  UniquePtr(const UniquePtr<U> &ptr)
    : pInstance(ptr.pInstance)
  {
    ptr.pInstance = nullptr;
  }

  ~UniquePtr()
  {
    internal::Destroy<T, std::is_base_of<RefCounted, T>::value>::destroy(pInstance);
  }

  UniquePtr &operator=(const UniquePtr<T> &ptr)
  {
    reset();
    pInstance = ptr.pInstance;
    ptr.pInstance = nullptr;
    return *this;
  }
  template<typename U>
  UniquePtr &operator=(const UniquePtr<U> &ptr)
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
inline RefCounted::~RefCounted()
{
  internal::NullifyWeakPtr(this);
}


namespace internal {

template<typename T>
class WeakProxy : public RefCounted
{
public:
  ~WeakProxy()
  {
    if (pInstance)
      internal::NullifyWeakPtr(pInstance);
  }
  T* pInstance;
};

} // namespace internal


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
  if (pInstance)
  {
    pInstance->rc = 1;
    ptr.pInstance = nullptr;
  }
}
template <class T>
template <class U>
SharedPtr<T>::SharedPtr(const WeakPtr<U> &ptr)
  : SharedPtr<T>(ptr.ptr())
{
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
    {
      delete pInstance;
    }
    pInstance = nullptr;
  }
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

template<typename T>
ptrdiff_t epStringify(Slice<char> buffer, String format, SharedPtr<T> spT, const epVarArg *pArgs)
{
  return epStringifyTemplate(buffer, format, spT.ptr(), pArgs);
}

// cast functions
template<class T, class U>
inline UniquePtr<T> unique_pointer_cast(const UniquePtr<U> &ptr)
{
  return UniquePtr<T>((T*)ptr.ptr());
}
template<class T, class U>
inline SharedPtr<T> shared_pointer_cast(const SharedPtr<U> &ptr)
{
  return SharedPtr<T>((T*)ptr.ptr());
}
template<class T, class U>
inline WeakPtr<T> weak_pointer_cast(const WeakPtr<U> &ptr)
{
  return WeakPtr<T>((T*)ptr.ptr());
}

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

template<class T, class U> inline bool operator==(const WeakPtr<T> &l, const WeakPtr<U> &r) { return l.ptr() == r.ptr(); }
template<class T, class U> inline bool operator!=(const WeakPtr<T> &l, const WeakPtr<U> &r) { return l.ptr() != r.ptr(); }
template<class T, class U> inline bool operator<=(const WeakPtr<T> &l, const WeakPtr<U> &r) { return l.ptr() <= r.ptr(); }
template<class T, class U> inline bool operator<(const WeakPtr<T> &l, const WeakPtr<U> &r) { return l.ptr() < r.ptr(); }
template<class T, class U> inline bool operator>=(const WeakPtr<T> &l, const WeakPtr<U> &r) { return l.ptr() >= r.ptr(); }
template<class T, class U> inline bool operator>(const WeakPtr<T> &l, const WeakPtr<U> &r) { return l.ptr() > r.ptr(); }
template<class T> inline bool operator==(const WeakPtr<T> &l, nullptr_t) { return l.ptr() == nullptr; }
template<class T> inline bool operator!=(const WeakPtr<T> &l, nullptr_t) { return l.ptr() != nullptr; }
template<class T> inline bool operator==(nullptr_t, const WeakPtr<T> &r) { return nullptr == r.ptr(); }
template<class T> inline bool operator!=(nullptr_t, const WeakPtr<T> &r) { return nullptr != r.ptr(); }

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

// unit tests
epResult epSharedPtr_Test();

#endif // _EPSHAREDPTR_HPP
