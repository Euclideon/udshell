#pragma once
#if !defined(_SHAREDPTR_H)

#define SHARED_CLASS(Name) \
  class Name; \
  typedef SharedPtr<Name> Name##Ref;

#define SHARED_STRUCT(Name) \
  struct Name; \
  typedef SharedPtr<Name> Name##Ref;

namespace udKernel
{

class RefCounted;

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
    if(this != &ptr)
      ptr.pInstance = nullptr;
  }

//  SharedPtr() {}        // <- should we allow default construction, or enforce nullptr?
  SharedPtr(nullptr_t) {}

  explicit SharedPtr(T *p) : pInstance(p) { acquire(); }

  inline ~SharedPtr() { release(); }

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
  SharedPtr& operator=(nullptr_t)
  {
    release();
    return *this;
  }

  inline void reset()
  {
    release();
  }

  inline operator bool() const
  {
    return count() > 0;
  }
  inline bool unique() const
  {
    return count() == 1;
  }
  inline size_t count() const;

  inline T& operator*() const { return *(T*)pInstance; }
  inline T* operator->() const { return (T*)pInstance; }
  inline T* ptr() const { return (T*)pInstance; }

private:
  template<class U> friend class SharedPtr;

  inline void acquire();
  inline void release();

  RefCounted *pInstance = nullptr;
};

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
};
inline RefCounted::~RefCounted() {}


template<class T>
class UniquePtr
{
public:
  UniquePtr() {}
  explicit UniquePtr(T *p) : pInstance(p) {}
  template <class U>
  UniquePtr(const UniquePtr<U> &ptr)
    : pInstance(static_cast<T*>(ptr.pInstance))
  {
    const_cast<UniquePtr<U>&>(ptr).pInstance = nullptr;
  }
  UniquePtr(const UniquePtr &ptr)
    : pInstance(ptr.pInstance)
  {
    const_cast<UniquePtr&>(ptr).pInstance = nullptr;
  }
  inline ~UniquePtr()
  {
    destroy();
  }

  UniquePtr &operator=(const UniquePtr &ptr)
  {
    destroy();
    pInstance = ptr.pInstance;
    const_cast<UniquePtr&>(ptr).pInstance = nullptr;
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
  inline operator bool() const { return pInstance != nullptr; }

  // underlying pointer operations :
  inline T& operator*() const { return *pInstance; }
  inline T* operator->() const { return pInstance; }
  inline T* ptr() const { return pInstance; }

private:
  template<typename U> friend class UniquePtr;

  inline void destroy()
  {
    delete pInstance;
    pInstance = nullptr;
  }
  inline void release() const
  {
    pInstance = nullptr;
  }

  T *pInstance = nullptr;
};


// cast functions
template<class T, class U>
SharedPtr<T> static_pointer_cast(const SharedPtr<U> &ptr)
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

template<class T, class U> inline bool operator==(const UniquePtr<T> &l, const UniquePtr<U> &r) { return l.ptr() == r.ptr(); }
template<class T, class U> inline bool operator!=(const UniquePtr<T> &l, const UniquePtr<U> &r) { return l.ptr() != r.ptr(); }
template<class T, class U> inline bool operator<=(const UniquePtr<T> &l, const UniquePtr<U> &r) { return l.ptr() <= r.ptr(); }
template<class T, class U> inline bool operator<(const UniquePtr<T> &l, const UniquePtr<U> &r) { return l.ptr() < r.ptr(); }
template<class T, class U> inline bool operator>=(const UniquePtr<T> &l, const UniquePtr<U> &r) { return l.ptr() >= r.ptr(); }
template<class T, class U> inline bool operator>(const UniquePtr<T> &l, const UniquePtr<U> &r) { return l.ptr() > r.ptr(); }



template<class T>
inline size_t SharedPtr<T>::count() const
{
  return pInstance ? pInstance->rc : 0;
}

template<class T>
inline void SharedPtr<T>::acquire()
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
} // namespace udKernel

#endif // _SHAREDPTR_H
