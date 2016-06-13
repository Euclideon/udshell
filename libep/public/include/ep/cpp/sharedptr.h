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

class RefCounted;
template<class T>
struct UniquePtr;


// **HAX** this allows us to delete a RefCounted!
// Some classes that derive off RefCounted have private/protected destructors. For ~UniquePtr() to be able to
// destroy a reference to one of these it has to cast to a RefCount* first to be able to
// call delete. Thee two specialisations of Destroy facilitate that.

namespace internal {

  template <typename T, typename U>
  struct CreateHelper;

  template<typename T, bool isref>
  struct Create;

  template<typename T>
  struct Create<T, false>
  {
    template <typename... Args>
    epforceinline static UniquePtr<T> create(Args&&... args);
  };

  template<typename T>
  struct Create<T, true>
  {
    template <typename... Args>
    epforceinline static UniquePtr<T> create(Args&&... args);
  };

  template<typename T, bool isref>
  struct Acquire;

  template<class T>
  struct Acquire<T, false>
  {
    epforceinline static void acquire(T *ptr);
  };

  template<class T>
  struct Acquire<T, true>
  {
    epforceinline static void acquire(T *ptr);
  };

  template<typename T, bool isref>
  struct Release;

  template<class T>
  struct Release<T, false>
  {
    epforceinline static void release(T *ptr);
  };

  template<class T>
  struct Release<T, true>
  {
    epforceinline static void release(T *ptr);
  };

} // namespace internal

class Safe
{
protected:
  virtual ~Safe();
};

// NOTE!! This is necessary because forward declarations from friend statements seem to declare stuff in the global scope with GCC
template<typename U> struct SafePtr;
template<typename U> struct SynchronisedPtr;

// shared pointers are ref counted
template<class T>
struct SharedPtr
{
public:
  using Type = T;

  // create a new instance of T
  template<typename... Args>
  static SharedPtr<T> create(Args&&... args)
  {
    static_assert(std::is_base_of<RefCounted, T>::value, "T does not derive from RefCounted");
    void *pMem = epAlloc(sizeof(T));
    EPTHROW_IF_NULL(pMem, Result::AllocFailure, "Memory allocation failed");
    epscope(fail) { epFree(pMem); };
    T *ptr = epConstruct(pMem) T(std::forward<Args>(args)...);
    ptr->pFreeFunc = [](RefCounted *pMem) { epFree((T*)pMem); };
    return SharedPtr<T>(ptr);
  }

  // This allows SharedPtr<const T>
  SharedPtr()
    : pInstance(nullptr)
  {
    static_assert(std::is_base_of<RefCounted, T>::value, "SharedPtr<T>(), T does not derive from RefCounted");
  }
  SharedPtr(nullptr_t)
    : pInstance(nullptr)
  {
    static_assert(std::is_base_of<RefCounted, T>::value, "SharedPtr<T>(nullptr_t), T does not derive from RefCounted");
  }

  // constructors
  SharedPtr(const SharedPtr<T> &ptr)
    : pInstance(acquire(ptr.ptr())) {}
  SharedPtr(SharedPtr<T> &&ptr)
    : pInstance(ptr.pInstance)
  {
    ptr.pInstance = nullptr;
  }

  // the U allows us to accept const
  template <class U>
  SharedPtr(const SharedPtr<U> &ptr)
    : pInstance(acquire(ptr.ptr())) {}
  template <class U>
  SharedPtr(SharedPtr<U> &&ptr)
  {
    static_assert(std::is_base_of<RefCounted, T>::value, "T does not derive from RefCounted");
    pInstance = ptr.pInstance;
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
    : pInstance(acquire(p))
  {
    EPASSERT(!p || p->pFreeFunc != nullptr, "Object has no 'free' function assigned!");
  }

  ~SharedPtr() { release(pInstance); }

  SharedPtr& operator=(SharedPtr<T> &&ptr)
  {
    if (pInstance != ptr.pInstance)
    {
      T *pOld = pInstance;
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
      T *pOld = pInstance;
      pInstance = acquire(ptr.ptr());
      release(pOld);
    }
    return *this;
  }
  template <class U> // the U allows us to accept const
  SharedPtr& operator=(const SharedPtr<U> &ptr)
  {
    if (pInstance != ptr.pInstance)
    {
      T *pOld = pInstance;
      pInstance = acquire(ptr.ptr());
      release(pOld);
    }
    return *this;
  }
  SharedPtr& operator=(UniquePtr<T> &&ptr);
  template <class U> // the U allows us to accept const
  SharedPtr& operator=(UniquePtr<U> &ptr);
  SharedPtr& operator=(nullptr_t)
  {
    T *pOld = pInstance;
    pInstance = nullptr;
    release(pOld);
    return *this;
  }

  template <class U>
  SharedPtr& operator=(SafePtr<U> &ptr)
  {
    T *pOld = pInstance;
    pInstance = acquire(ptr.ptr());
    release(pOld);
    return *this;
  }

  void reset()
  {
    T *pOld = pInstance;
    pInstance = nullptr;
    release(pOld);
  }

  size_t count() const;

  epforceinline bool unique() const { return count() == 1; }

  epforceinline explicit operator bool() const { return count() > 0; }

  T& operator*() const { return *pInstance; }
  T* operator->() const { return pInstance; }
  T* ptr() const { return pInstance; }

private:
  template<typename U> friend struct UniquePtr;
  template<typename U> friend struct SharedPtr;
  template<typename U> friend struct SafePtr;
  template<typename U> friend struct SynchronisedPtr;

  static T* acquire(T *pI);
  static void release(T *pI);

  T *pInstance = nullptr;
};

//------------------------------------------------------------------------------------------

// unique pointers nullify the source pointer on assignment
template<typename T>
struct UniquePtr
{
public:
  using Type = T;

  // create a new instance of T
  template<typename... Args>
  static UniquePtr<T> create(Args&&... args)
  {
    return internal::Create<T, std::is_base_of<RefCounted, T>::value>::create(std::forward<Args>(args)...);
  }

  UniquePtr() {}
  UniquePtr(nullptr_t) {}

  // NOTE: This function is potentially dangerous. If it is passed a pointer to something that wasn't allocated from our heap it will result in a crash on the call to delete.
  explicit UniquePtr(T *p) : pInstance(p)
  {
    internal::Acquire<T, std::is_base_of<RefCounted, T>::value>::acquire(pInstance);
  }

  UniquePtr(const UniquePtr<T> &ptr) = delete;
  UniquePtr &operator=(const UniquePtr<T> &ptr) = delete;

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
    internal::Release<T, std::is_base_of<RefCounted, T>::value>::release(pInstance);
  }

  UniquePtr &operator=(UniquePtr<T> &&ptr)
  {
    if (this != &ptr)
    {
      reset();
      pInstance = ptr.pInstance;
      ptr.pInstance = nullptr;
    }
    return *this;
  }
  template<typename U>
  UniquePtr &operator=(UniquePtr<U> &&ptr)
  {
    if (this != &ptr)
    {
      reset();
      pInstance = ptr.pInstance;
      ptr.pInstance = nullptr;
    }
    return *this;
  }

  void reset()
  {
    this->~UniquePtr();
    pInstance = nullptr;
  }

  // reference counter operations:
  explicit operator bool() const { return pInstance != nullptr; }

  // underlying pointer operations:
  T& operator*() const { return *pInstance; }
  T* operator->() const { return pInstance; }
  T* ptr() const { return pInstance; }

private:
  template<typename U> friend struct SharedPtr;
  template<typename U> friend struct UniquePtr;
  template<typename U, bool isref> friend struct Acquire;
  template<typename U, bool isref> friend struct Release;

  T *pInstance = nullptr;
};


// ref counting base class
class RefCounted : public Safe
{
  void Destroy();

public:
  size_t RefCount() const { return rc; }
  size_t IncRef() { return ++rc; }
  size_t DecRef()
  {
    if (rc == 1)
    {
      FreeFunc *pFree = pFreeFunc;
      this->~RefCounted();
      pFree(this);
      return 0;
    }
    return --rc;
  }

  template<typename T, typename... Args>
  static T* New(Args... args)
  {
    static_assert(std::is_base_of<RefCounted, T>::value, "T does not derive from RefCounted");
    void *pMem = epAlloc(sizeof(T));
    EPTHROW_IF_NULL(pMem, Result::AllocFailure, "Memory allocation failed");
    epscope(fail) { epFree(pMem); };
    T *ptr = epConstruct(pMem) T(std::forward<Args>(args)...);
    ptr->pFreeFunc = [](RefCounted *pMem) { epFree((T*)pMem); };
    return ptr;
  }

private:
  typedef void (FreeFunc)(RefCounted*);

  mutable size_t rc = 0;
  FreeFunc *pFreeFunc = nullptr;

  // friends to init pFreeFunc...
  template<typename T>
  friend struct SharedPtr;
  template<typename T, bool isrc>
  friend struct internal::Create;
  friend class Kernel;
  friend class KernelImpl;
  template<typename T, typename U>
  friend struct internal::CreateHelper;
};

// SharedPtr constructors and assignments
template <class T>
inline SharedPtr<T>::SharedPtr(UniquePtr<T> &&ptr)
  : pInstance(ptr.pInstance)
{
  ptr.pInstance = nullptr;
}

template <class T>
template <class U>
inline SharedPtr<T>::SharedPtr(UniquePtr<U> &ptr)
{
  static_assert(std::is_base_of<RefCounted, T>::value, "T does not derive from RefCounted");
  pInstance = ptr.pInstance;
  ptr.pInstance = nullptr;
}

template <class T>
inline SharedPtr<T>& SharedPtr<T>::operator=(UniquePtr<T> &&ptr)
{
  T *pOld = pInstance;
  pInstance = ptr.pInstance;
  ptr.pInstance = nullptr;
  release(pOld);
  return *this;
}

template <class T>
template <class U>
inline SharedPtr<T>& SharedPtr<T>::operator=(UniquePtr<U> &ptr)
{
  static_assert(std::is_base_of<RefCounted, T>::value, "U does not derive from T");
  T *pOld = pInstance;
  pInstance = ptr.pInstance;
  ptr.pInstance = nullptr;
  release(pOld);
  return *this;
}

template<class T>
epforceinline size_t SharedPtr<T>::count() const
{
  return pInstance ? pInstance->RefCount() : 0;
}

template<class T>
epforceinline T* SharedPtr<T>::acquire(T *pI)
{
  auto *pRC = (RefCounted*)pI; // we have already validated in constructors that T is RefCounted
  if (pRC)
    pRC->IncRef();
  return pI;
}
template<class T>
inline void SharedPtr<T>::release(T *pI)
{
  auto *pRC = (RefCounted*)pI;
  if (pRC)
    pRC->DecRef();
}

namespace internal {

  template <class T>
  template <typename... Args>
  epforceinline UniquePtr<T> Create<T, false>::create(Args&&... args)
  {
    return UniquePtr<T>(epNew(T,std::forward<Args>(args)...));
  }
  template <class T>
  template <typename... Args>
  epforceinline UniquePtr<T> Create<T, true>::create(Args&&... args)
  {
    void *pMem = epAlloc(sizeof(T));
    EPTHROW_IF_NULL(pMem, Result::AllocFailure, "Memory allocation failed");
    epscope(fail) { epFree(pMem); };
    using U = typename std::remove_const<T>::type;
    UniquePtr<U> up(epConstruct(pMem) U(std::forward<Args>(args)...));
    up->pFreeFunc = [](RefCounted *pMem) { epFree((U*)pMem); };
    return std::move(up);
  }

  template<class T>
  epforceinline void Acquire<T, false>::acquire(T*)
  {}
  template<class T>
  epforceinline void Acquire<T, true>::acquire(T *ptr)
  {
    if (ptr)
      ptr->IncRef();
  }

  template<class T>
  epforceinline void Release<T, false>::release(T *ptr)
  {
    epDelete(ptr);
  }
  template<class T>
  epforceinline void Release<T, true>::release(T *ptr)
  {
    if (ptr)
    {
      using U = typename std::remove_const<T>::type;
      const_cast<U*>(ptr)->DecRef();
    }
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
