#pragma once
#if !defined(_SHAREDPTR_H)
#define _SHAREDPTR_H

#include <type_traits>

#if defined(_MSC_VER)
// TODO: REMOVE THIS!!!
#pragma warning(disable: 4714)
#endif

#define SHARED_CLASS(Name) \
  class Name; \
  typedef udSharedPtr<Name> Name##Ref;

#define SHARED_STRUCT(Name) \
  struct Name; \
  typedef udSharedPtr<Name> Name##Ref;

class udRefCounted;
template<class T>
class udUniquePtr;


// shared pointers are ref counted
template<class T>
class udSharedPtr
{
public:
  // create a new instance of T
  template<typename... Args>
  static udSharedPtr<T> create(Args... args)
  {
    return udSharedPtr<T>(new T(args...));
  }

  // constructors
  udSharedPtr(const udSharedPtr<T> &ptr) : pInstance(ptr.pInstance) { acquire(); }
  template <class U> // the U allows us to accept const
  udSharedPtr(const udSharedPtr<U> &ptr) : pInstance(ptr.pInstance) { acquire(); }
  udSharedPtr(udSharedPtr<T> &&ptr)
    : pInstance(ptr.pInstance)
  {
    if(this != &ptr)
      ptr.pInstance = nullptr;
  }
  template <class U> // the U allows us to accept const
  udSharedPtr(const udUniquePtr<U> &ptr);
  udSharedPtr(udUniquePtr<T> &&ptr);

  udSharedPtr() {}
  udSharedPtr(nullptr_t) {}

  explicit udSharedPtr(T *p) : pInstance(p) { acquire(); }

  inline ~udSharedPtr() { release(); }

  udSharedPtr& operator=(udSharedPtr<T> &&ptr)
  {
    if (pInstance != ptr.pInstance)
    {
      release();
      pInstance = ptr.pInstance;
      ptr.pInstance = nullptr;
    }
    return *this;
  }
  udSharedPtr& operator=(const udSharedPtr<T> &ptr)
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
  udSharedPtr& operator=(const udSharedPtr<U> &ptr)
  {
    if (pInstance != ptr.pInstance)
    {
      release();
      pInstance = ptr.pInstance;
      acquire();
    }
    return *this;
  }
  udSharedPtr& operator=(udUniquePtr<T> &&ptr);
  template <class U> // the U allows us to accept const
  udSharedPtr& operator=(const udUniquePtr<U> &ptr);
  udSharedPtr& operator=(nullptr_t)
  {
    release();
    return *this;
  }

  inline void reset()
  {
    release();
  }

  inline size_t count() const;

  UDFORCE_INLINE bool unique() const { return count() == 1; }

  UDFORCE_INLINE explicit operator bool() const { return count() > 0; }

  UDFORCE_INLINE T& operator*() const { return *(T*)pInstance; }
  UDFORCE_INLINE T* operator->() const { return (T*)pInstance; }
  UDFORCE_INLINE T* ptr() const { return (T*)pInstance; }

private:
  template<class U> friend class udSharedPtr;

  inline void acquire();
  inline void release();

  udRefCounted *pInstance = nullptr;
};

//------------------------------------------------------------------------------------------

// **HAX** this crap allows us to delete a udRefCounted!
namespace internal {
  template<typename T, bool isref>
  struct Destroy;
  template<class T>
  struct Destroy<T, false> {
    UDFORCE_INLINE static void destroy(T *ptr)
    {
      delete ptr;
    }
  };
  template<class T>
  struct Destroy<T, true> {
    UDFORCE_INLINE static void destroy(T *ptr)
    {
      udRefCounted *rc = ptr;
      delete rc;
    }
  };
};

// unique pointers nullify the source pointer on assignment
template<class T>
class udUniquePtr
{
public:
  udUniquePtr() {}
  explicit udUniquePtr(T *p) : pInstance(p) {}
  template <class U>
  udUniquePtr(const udUniquePtr<U> &ptr)
    : pInstance(ptr.pInstance)
  {
    ptr.pInstance = nullptr;
  }
  udUniquePtr(const udUniquePtr &ptr)
    : pInstance(ptr.pInstance)
  {
    ptr.pInstance = nullptr;
  }
  ~udUniquePtr()
  {
    internal::Destroy<T, std::is_base_of<udRefCounted, T>::value>::destroy(pInstance);
  }

  udUniquePtr &operator=(const udUniquePtr &ptr)
  {
    reset();
    pInstance = ptr.pInstance;
    ptr.pInstance = nullptr;
    return *this;
  }

  inline void reset()
  {
    this->~udUniquePtr();
    pInstance = nullptr;
  }

  inline void release()
  {
    pInstance = nullptr;
  }

  // reference counter operations :
  UDFORCE_INLINE explicit operator bool() const { return pInstance != nullptr; }

  // underlying pointer operations :
  UDFORCE_INLINE T& operator*() const { return *pInstance; }
  UDFORCE_INLINE T* operator->() const { return pInstance; }
  UDFORCE_INLINE T* ptr() const { return pInstance; }

private:
  template<typename U> friend class udSharedPtr;
  template<typename U> friend class udUniquePtr;
  template<typename U, bool isref> friend struct Destroy;

  mutable T *pInstance = nullptr;
};

//------------------------------------------------------------------------------------------

// unique pointers that can be passed between threads, destruction is deferred to main thread
namespace ud { class Kernel; }

template<class T>
class udSynchronisedPtr
{
public:
  udSynchronisedPtr() {}
  template <class U>
  explicit udSynchronisedPtr(const udSharedPtr<U> &p, ud::Kernel *pKernel)
    : pInstance(p.ptr())
    , pKernel(pKernel)
  {
    p.acquire();
  }
  template <class U>
  udSynchronisedPtr(const udSynchronisedPtr<U> &ptr)
    : pInstance(ptr.pInstance)
    , pKernel(ptr.pKernel)
  {
    const_cast<udSynchronisedPtr<U>&>(ptr).pInstance = nullptr;
    const_cast<udSynchronisedPtr<U>&>(ptr).pKernel = nullptr;
  }
  udSynchronisedPtr(const udSynchronisedPtr &ptr)
    : pInstance(ptr.pInstance)
    , pKernel(ptr.pKernel)
  {
    const_cast<udSynchronisedPtr&>(ptr).pInstance = nullptr;
    const_cast<udSynchronisedPtr&>(ptr).pKernel = nullptr;
  }
  inline ~udSynchronisedPtr()
  {
    destroy();
  }

  udSynchronisedPtr &operator=(const udSynchronisedPtr &ptr)
  {
    destroy();
    pInstance = ptr.pInstance;
    pKernel = ptr.pKernel;
    const_cast<udSynchronisedPtr&>(ptr).pInstance = nullptr;
    const_cast<udSynchronisedPtr&>(ptr).pKernel = nullptr;
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
  template<typename U> friend class udSynchronisedPtr;

  void destroy();

  T *pInstance = nullptr;
  ud::Kernel *pKernel = nullptr;
};


// ref counting base class
class udRefCounted
{
  mutable size_t rc = 0;

protected:
  virtual ~udRefCounted() = 0;
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
  friend class udSharedPtr;
  template<typename T, bool isrc>
  friend struct internal::Destroy;
};
inline udRefCounted::~udRefCounted() {}


// cast functions
template<class T, class U>
inline udSharedPtr<T> shared_pointer_cast(const udSharedPtr<U> &ptr)
{
  return udSharedPtr<T>((T*)ptr.ptr());
}


// comparaison operators
template<class T, class U> inline bool operator==(const udSharedPtr<T> &l, const udSharedPtr<U> &r) { return l.ptr() == r.ptr(); }
template<class T, class U> inline bool operator!=(const udSharedPtr<T> &l, const udSharedPtr<U> &r) { return l.ptr() != r.ptr(); }
template<class T, class U> inline bool operator<=(const udSharedPtr<T> &l, const udSharedPtr<U> &r) { return l.ptr() <= r.ptr(); }
template<class T, class U> inline bool operator<(const udSharedPtr<T> &l, const udSharedPtr<U> &r) { return l.ptr() < r.ptr(); }
template<class T, class U> inline bool operator>=(const udSharedPtr<T> &l, const udSharedPtr<U> &r) { return l.ptr() >= r.ptr(); }
template<class T, class U> inline bool operator>(const udSharedPtr<T> &l, const udSharedPtr<U> &r) { return l.ptr() > r.ptr(); }
template<class T, class U> inline bool operator==(const udSharedPtr<T> &l, nullptr_t) { return l.ptr() == nullptr; }
template<class T, class U> inline bool operator!=(const udSharedPtr<T> &l, nullptr_t) { return l.ptr() != nullptr; }
template<class T, class U> inline bool operator==(nullptr_t, const udSharedPtr<U> &r) { return nullptr == r.ptr(); }
template<class T, class U> inline bool operator!=(nullptr_t, const udSharedPtr<U> &r) { return nullptr != r.ptr(); }

template<class T, class U> inline bool operator==(const udUniquePtr<T> &l, const udUniquePtr<U> &r) { return l.ptr() == r.ptr(); }
template<class T, class U> inline bool operator!=(const udUniquePtr<T> &l, const udUniquePtr<U> &r) { return l.ptr() != r.ptr(); }
template<class T, class U> inline bool operator<=(const udUniquePtr<T> &l, const udUniquePtr<U> &r) { return l.ptr() <= r.ptr(); }
template<class T, class U> inline bool operator<(const udUniquePtr<T> &l, const udUniquePtr<U> &r) { return l.ptr() < r.ptr(); }
template<class T, class U> inline bool operator>=(const udUniquePtr<T> &l, const udUniquePtr<U> &r) { return l.ptr() >= r.ptr(); }
template<class T, class U> inline bool operator>(const udUniquePtr<T> &l, const udUniquePtr<U> &r) { return l.ptr() > r.ptr(); }
template<class T, class U> inline bool operator==(const udUniquePtr<T> &l, nullptr_t) { return l.ptr() == nullptr; }
template<class T, class U> inline bool operator!=(const udUniquePtr<T> &l, nullptr_t) { return l.ptr() != nullptr; }
template<class T, class U> inline bool operator==(nullptr_t, const udUniquePtr<U> &r) { return nullptr == r.ptr(); }
template<class T, class U> inline bool operator!=(nullptr_t, const udUniquePtr<U> &r) { return nullptr != r.ptr(); }


// udSharedPtr constructors and assignments
template <class T>
inline udSharedPtr<T>::udSharedPtr(udUniquePtr<T> &&ptr)
  : pInstance(ptr.pInstance)
{
  pInstance->rc = 1;
  ptr.pInstance = nullptr;
}
template <class T>
template <class U>
inline udSharedPtr<T>::udSharedPtr(const udUniquePtr<U> &ptr)
  : pInstance(ptr.pInstance)
{
  pInstance->rc = 1;
  ptr.pInstance = nullptr;
}
template <class T>
inline udSharedPtr<T>& udSharedPtr<T>::operator=(udUniquePtr<T> &&ptr)
{
  release();
  pInstance = ptr.pInstance;
  pInstance->rc = 1;
  ptr.pInstance = nullptr;
  return *this;
}
template <class T>
template <class U>
inline udSharedPtr<T>& udSharedPtr<T>::operator=(const udUniquePtr<U> &ptr)
{
  release();
  pInstance = ptr.pInstance;
  pInstance->rc = 1;
  ptr.pInstance = nullptr;
  return *this;
}

template<class T>
UDFORCE_INLINE size_t udSharedPtr<T>::count() const
{
  return pInstance ? pInstance->rc : 0;
}

template<class T>
UDFORCE_INLINE void udSharedPtr<T>::acquire()
{
  if (pInstance)
  {
    ++pInstance->rc;
  }
}
template<class T>
inline void udSharedPtr<T>::release()
{
  if (pInstance)
  {
    if (--pInstance->rc == 0)
      delete pInstance;
    pInstance = nullptr;
  }
}

#endif // _SHAREDPTR_H
