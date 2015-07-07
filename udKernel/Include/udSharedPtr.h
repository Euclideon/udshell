#pragma once
#if !defined(_SHAREDPTR_H)

class udRefCounted;

template<class T>
class udSharedPtr
{
public:
  udSharedPtr() {}

  explicit udSharedPtr(T *p) : pInstance(p) { acquire(); }
  udSharedPtr(const udSharedPtr<T> &ptr) : pInstance(ptr.pInstance) { acquire(); }
  template <class U> // the U allows us to accept const
  udSharedPtr(const udSharedPtr<U> &ptr) : pInstance(ptr.pInstance) { acquire(); }
  inline ~udSharedPtr() { release(); }

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
  size_t count() const
  {
    return pInstance ? ((udRefCounted*)pInstance)->rc : 0;
  }

  inline T& operator*() const { return *pInstance; }
  inline T* operator->() const { return pInstance; }
  inline T* ptr() const { return pInstance; }

private:
  template<class U> friend class udSharedPtr;

  inline void acquire()
  {
    if (pInstance)
    {
      udRefCounted *pRC = (udRefCounted*)pInstance;
      ++pRC->rc;
    }
  }
  inline void release()
  {
    if (pInstance)
    {
      udRefCounted *pRC = (udRefCounted*)pInstance;
      if (--pRC->rc == 0)
        delete pRC;
      pInstance = nullptr;
    }
  }

  T *pInstance = nullptr;
};

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
};
inline udRefCounted::~udRefCounted() {}


template<class T>
class udUniquePtr
{
public:
  udUniquePtr() {}
  explicit udUniquePtr(T *p) : pInstance(p) {}
  template <class U>
  udUniquePtr(const udUniquePtr<U> &ptr)
    : pInstance(static_cast<T*>(ptr.pInstance))
  {
    const_cast<udUniquePtr<U>&>(ptr).pInstance = nullptr;
  }
  udUniquePtr(const udUniquePtr &ptr)
    : pInstance(ptr.pInstance)
  {
    const_cast<udUniquePtr&>(ptr).pInstance = nullptr;
  }
  inline ~udUniquePtr()
  {
    destroy();
  }

  udUniquePtr &operator=(const udUniquePtr &ptr)
  {
    destroy();
    pInstance = ptr.pInstance;
    const_cast<udUniquePtr&>(ptr).pInstance = nullptr;
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
  template<typename U> friend class udUniquePtr;

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
udSharedPtr<T> static_pointer_cast(const udSharedPtr<U> &ptr)
{
  return (udSharedPtr<T>&)ptr;
}


// comparaison operators
template<class T, class U> inline bool operator==(const udSharedPtr<T> &l, const udSharedPtr<U> &r) { return l.ptr() == r.ptr(); }
template<class T, class U> inline bool operator!=(const udSharedPtr<T> &l, const udSharedPtr<U> &r) { return l.ptr() != r.ptr(); }
template<class T, class U> inline bool operator<=(const udSharedPtr<T> &l, const udSharedPtr<U> &r) { return l.ptr() <= r.ptr(); }
template<class T, class U> inline bool operator<(const udSharedPtr<T> &l, const udSharedPtr<U> &r) { return l.ptr() < r.ptr(); }
template<class T, class U> inline bool operator>=(const udSharedPtr<T> &l, const udSharedPtr<U> &r) { return l.ptr() >= r.ptr(); }
template<class T, class U> inline bool operator>(const udSharedPtr<T> &l, const udSharedPtr<U> &r) { return l.ptr() > r.ptr(); }

template<class T, class U> inline bool operator==(const udUniquePtr<T> &l, const udUniquePtr<U> &r) { return l.ptr() == r.ptr(); }
template<class T, class U> inline bool operator!=(const udUniquePtr<T> &l, const udUniquePtr<U> &r) { return l.ptr() != r.ptr(); }
template<class T, class U> inline bool operator<=(const udUniquePtr<T> &l, const udUniquePtr<U> &r) { return l.ptr() <= r.ptr(); }
template<class T, class U> inline bool operator<(const udUniquePtr<T> &l, const udUniquePtr<U> &r) { return l.ptr() < r.ptr(); }
template<class T, class U> inline bool operator>=(const udUniquePtr<T> &l, const udUniquePtr<U> &r) { return l.ptr() >= r.ptr(); }
template<class T, class U> inline bool operator>(const udUniquePtr<T> &l, const udUniquePtr<U> &r) { return l.ptr() > r.ptr(); }

#endif // _SHAREDPTR_H
