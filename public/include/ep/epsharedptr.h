#pragma once
#if !defined(_EPSHAREDPTR_H)
#define _EPSHAREDPTR_H

#include <type_traits>
#include "ep/epplatform.h"

#if defined(EP_COMPILER_VISUALC)
// TODO: REMOVE THIS!!!
#pragma warning(disable: 4714)
#endif

#define SHARED_CLASS(Name) \
  class Name; \
  typedef epSharedPtr<Name> Name##Ref;

#define SHARED_STRUCT(Name) \
  struct Name; \
  typedef epSharedPtr<Name> Name##Ref;

class epRefCounted;
template<class T>
class epUniquePtr;


// weak pointer is just an alias for obvious annotation in code, may be fleshed out for debug at some point...
template<typename T, typename std::enable_if<std::is_base_of<epRefCounted, T>::value>::type* = nullptr>
using epWeakPtr = T*;


// shared pointers are ref counted
template<class T>
class epSharedPtr
{
public:
  // create a new instance of T
  template<typename... Args>
  static epSharedPtr<T> create(Args... args)
  {
    return epSharedPtr<T>(new T(args...));
  }

  // constructors
  epSharedPtr(const epSharedPtr<T> &ptr) : pInstance(ptr.pInstance) { acquire(); }
  template <class U> // the U allows us to accept const
  epSharedPtr(const epSharedPtr<U> &ptr) : pInstance(ptr.pInstance) { acquire(); }
  epSharedPtr(epSharedPtr<T> &&ptr)
    : pInstance(ptr.pInstance)
  {
    if(this != &ptr)
      ptr.pInstance = nullptr;
  }
  template <class U> // the U allows us to accept const
  epSharedPtr(const epUniquePtr<U> &ptr);
  epSharedPtr(epUniquePtr<T> &&ptr);

  epSharedPtr() {}
  epSharedPtr(nullptr_t) {}

  explicit epSharedPtr(T *p) : pInstance(p) { acquire(); }

  inline ~epSharedPtr() { release(); }

  epSharedPtr& operator=(epSharedPtr<T> &&ptr)
  {
    if (pInstance != ptr.pInstance)
    {
      release();
      pInstance = ptr.pInstance;
      ptr.pInstance = nullptr;
    }
    return *this;
  }
  epSharedPtr& operator=(const epSharedPtr<T> &ptr)
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
  epSharedPtr& operator=(const epSharedPtr<U> &ptr)
  {
    if (pInstance != ptr.pInstance)
    {
      release();
      pInstance = ptr.pInstance;
      acquire();
    }
    return *this;
  }
  epSharedPtr& operator=(epUniquePtr<T> &&ptr);
  template <class U> // the U allows us to accept const
  epSharedPtr& operator=(const epUniquePtr<U> &ptr);
  epSharedPtr& operator=(nullptr_t)
  {
    release();
    return *this;
  }

  inline void reset()
  {
    release();
  }

  inline size_t count() const;

  EPALWAYS_INLINE bool unique() const { return count() == 1; }

  EPALWAYS_INLINE explicit operator bool() const { return count() > 0; }

  EPALWAYS_INLINE T& operator*() const { return *(T*)pInstance; }
  EPALWAYS_INLINE T* operator->() const { return (T*)pInstance; }
  EPALWAYS_INLINE T* ptr() const { return (T*)pInstance; }

private:
  template<class U> friend class epSharedPtr;

  inline void acquire();
  inline void release();

  epRefCounted *pInstance = nullptr;
};

//------------------------------------------------------------------------------------------

// **HAX** this crap allows us to delete a epRefCounted!
namespace ep {
namespace internal {

template<typename T, bool isref>
struct Destroy;
template<class T>
struct Destroy<T, false> {
  EPALWAYS_INLINE static void destroy(T *ptr)
  {
    delete ptr;
  }
};
template<class T>
struct Destroy<T, true> {
  EPALWAYS_INLINE static void destroy(T *ptr)
  {
    epRefCounted *rc = ptr;
    delete rc;
  }
};

} // namespace internal
} // namespace ep

// unique pointers nullify the source pointer on assignment
template<class T>
class epUniquePtr
{
public:
  epUniquePtr() {}
  explicit epUniquePtr(T *p) : pInstance(p) {}
  template <class U>
  epUniquePtr(const epUniquePtr<U> &ptr)
    : pInstance(ptr.pInstance)
  {
    ptr.pInstance = nullptr;
  }
  epUniquePtr(const epUniquePtr &ptr)
    : pInstance(ptr.pInstance)
  {
    ptr.pInstance = nullptr;
  }
  ~epUniquePtr()
  {
    ep::internal::Destroy<T, std::is_base_of<epRefCounted, T>::value>::destroy(pInstance);
  }

  epUniquePtr &operator=(const epUniquePtr &ptr)
  {
    reset();
    pInstance = ptr.pInstance;
    ptr.pInstance = nullptr;
    return *this;
  }

  inline void reset()
  {
    this->~epUniquePtr();
    pInstance = nullptr;
  }

  inline void release()
  {
    pInstance = nullptr;
  }

  // reference counter operations :
  EPALWAYS_INLINE explicit operator bool() const { return pInstance != nullptr; }

  // underlying pointer operations :
  EPALWAYS_INLINE T& operator*() const { return *pInstance; }
  EPALWAYS_INLINE T* operator->() const { return pInstance; }
  EPALWAYS_INLINE T* ptr() const { return pInstance; }

private:
  template<typename U> friend class epSharedPtr;
  template<typename U> friend class epUniquePtr;
  template<typename U, bool isref> friend struct Destroy;

  mutable T *pInstance = nullptr;
};

//------------------------------------------------------------------------------------------

// unique pointers that can be passed between threads, destruction is deferred to main thread
namespace ep { class Kernel; }

template<class T>
class epSynchronisedPtr
{
public:
  epSynchronisedPtr() {}
  template <class U>
  explicit epSynchronisedPtr(const epSharedPtr<U> &p, ep::Kernel *pKernel)
    : pInstance(p.ptr())
    , pKernel(pKernel)
  {
    p.acquire();
  }
  template <class U>
  epSynchronisedPtr(const epSynchronisedPtr<U> &ptr)
    : pInstance(ptr.pInstance)
    , pKernel(ptr.pKernel)
  {
    const_cast<epSynchronisedPtr<U>&>(ptr).pInstance = nullptr;
    const_cast<epSynchronisedPtr<U>&>(ptr).pKernel = nullptr;
  }
  epSynchronisedPtr(const epSynchronisedPtr &ptr)
    : pInstance(ptr.pInstance)
    , pKernel(ptr.pKernel)
  {
    const_cast<epSynchronisedPtr&>(ptr).pInstance = nullptr;
    const_cast<epSynchronisedPtr&>(ptr).pKernel = nullptr;
  }
  inline ~epSynchronisedPtr()
  {
    destroy();
  }

  epSynchronisedPtr &operator=(const epSynchronisedPtr &ptr)
  {
    destroy();
    pInstance = ptr.pInstance;
    pKernel = ptr.pKernel;
    const_cast<epSynchronisedPtr&>(ptr).pInstance = nullptr;
    const_cast<epSynchronisedPtr&>(ptr).pKernel = nullptr;
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
  template<typename U> friend class epSynchronisedPtr;

  void destroy();

  T *pInstance = nullptr;
  ep::Kernel *pKernel = nullptr;
};


// ref counting base class
class epRefCounted
{
  mutable size_t rc = 0;

protected:
  virtual ~epRefCounted() = 0;
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
  friend class epSharedPtr;
  template<typename T, bool isrc>
  friend struct ep::internal::Destroy;
};
inline epRefCounted::~epRefCounted() {}


// cast functions
template<class T, class U>
inline epSharedPtr<T> shared_pointer_cast(const epSharedPtr<U> &ptr)
{
  return epSharedPtr<T>((T*)ptr.ptr());
}


// comparaison operators
template<class T, class U> inline bool operator==(const epSharedPtr<T> &l, const epSharedPtr<U> &r) { return l.ptr() == r.ptr(); }
template<class T, class U> inline bool operator!=(const epSharedPtr<T> &l, const epSharedPtr<U> &r) { return l.ptr() != r.ptr(); }
template<class T, class U> inline bool operator<=(const epSharedPtr<T> &l, const epSharedPtr<U> &r) { return l.ptr() <= r.ptr(); }
template<class T, class U> inline bool operator<(const epSharedPtr<T> &l, const epSharedPtr<U> &r) { return l.ptr() < r.ptr(); }
template<class T, class U> inline bool operator>=(const epSharedPtr<T> &l, const epSharedPtr<U> &r) { return l.ptr() >= r.ptr(); }
template<class T, class U> inline bool operator>(const epSharedPtr<T> &l, const epSharedPtr<U> &r) { return l.ptr() > r.ptr(); }
template<class T, class U> inline bool operator==(const epSharedPtr<T> &l, nullptr_t) { return l.ptr() == nullptr; }
template<class T, class U> inline bool operator!=(const epSharedPtr<T> &l, nullptr_t) { return l.ptr() != nullptr; }
template<class T, class U> inline bool operator==(nullptr_t, const epSharedPtr<U> &r) { return nullptr == r.ptr(); }
template<class T, class U> inline bool operator!=(nullptr_t, const epSharedPtr<U> &r) { return nullptr != r.ptr(); }

template<class T, class U> inline bool operator==(const epUniquePtr<T> &l, const epUniquePtr<U> &r) { return l.ptr() == r.ptr(); }
template<class T, class U> inline bool operator!=(const epUniquePtr<T> &l, const epUniquePtr<U> &r) { return l.ptr() != r.ptr(); }
template<class T, class U> inline bool operator<=(const epUniquePtr<T> &l, const epUniquePtr<U> &r) { return l.ptr() <= r.ptr(); }
template<class T, class U> inline bool operator<(const epUniquePtr<T> &l, const epUniquePtr<U> &r) { return l.ptr() < r.ptr(); }
template<class T, class U> inline bool operator>=(const epUniquePtr<T> &l, const epUniquePtr<U> &r) { return l.ptr() >= r.ptr(); }
template<class T, class U> inline bool operator>(const epUniquePtr<T> &l, const epUniquePtr<U> &r) { return l.ptr() > r.ptr(); }
template<class T, class U> inline bool operator==(const epUniquePtr<T> &l, nullptr_t) { return l.ptr() == nullptr; }
template<class T, class U> inline bool operator!=(const epUniquePtr<T> &l, nullptr_t) { return l.ptr() != nullptr; }
template<class T, class U> inline bool operator==(nullptr_t, const epUniquePtr<U> &r) { return nullptr == r.ptr(); }
template<class T, class U> inline bool operator!=(nullptr_t, const epUniquePtr<U> &r) { return nullptr != r.ptr(); }


// epSharedPtr constructors and assignments
template <class T>
inline epSharedPtr<T>::epSharedPtr(epUniquePtr<T> &&ptr)
  : pInstance(ptr.pInstance)
{
  pInstance->rc = 1;
  ptr.pInstance = nullptr;
}
template <class T>
template <class U>
inline epSharedPtr<T>::epSharedPtr(const epUniquePtr<U> &ptr)
  : pInstance(ptr.pInstance)
{
  pInstance->rc = 1;
  ptr.pInstance = nullptr;
}
template <class T>
inline epSharedPtr<T>& epSharedPtr<T>::operator=(epUniquePtr<T> &&ptr)
{
  release();
  pInstance = ptr.pInstance;
  pInstance->rc = 1;
  ptr.pInstance = nullptr;
  return *this;
}
template <class T>
template <class U>
inline epSharedPtr<T>& epSharedPtr<T>::operator=(const epUniquePtr<U> &ptr)
{
  release();
  pInstance = ptr.pInstance;
  pInstance->rc = 1;
  ptr.pInstance = nullptr;
  return *this;
}

template<class T>
EPALWAYS_INLINE size_t epSharedPtr<T>::count() const
{
  return pInstance ? pInstance->rc : 0;
}

template<class T>
EPALWAYS_INLINE void epSharedPtr<T>::acquire()
{
  if (pInstance)
  {
    ++pInstance->rc;
  }
}
template<class T>
inline void epSharedPtr<T>::release()
{
  if (pInstance)
  {
    if (--pInstance->rc == 0)
      delete pInstance;
    pInstance = nullptr;
  }
}

#endif // _EPSHAREDPTR_H
