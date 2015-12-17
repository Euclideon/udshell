#pragma once
#if !defined(_EPSAFEPTR_HPP)
#define _EPSAFEPTR_HPP

#include "ep/cpp/sharedptr.h"

namespace ep {

// **HAX** this allows us to delete a RefCounted!
// Some classes that derive off RefCounted have private/protected destructors. For ~UniquePtr() to be able to
// destroy a reference to one of these it has to cast to a RefCount* first to be able to
// call delete. Thee two specialisations of Destroy facilitate that.

namespace internal {

  template <class T>
  class SafeProxy;

  void* GetSafePtr(void *pAlloc);
  void NullifySafePtr(void *pAlloc);

} // namespace internal


template <typename T>
class SafePtr
{
public:
  SafePtr(const SafePtr<T> &ptr) : spProxy(ptr.spProxy) {}
  SafePtr(SafePtr<T> &&ptr) : spProxy(std::move(ptr.spProxy)) {}

  // the U allows us to accept const
  template <class U>
  SafePtr(const SafePtr<U> &ptr) : spProxy(ptr.spProxy) {}
  template <class U>
  SafePtr(SafePtr<U> &&ptr) : spProxy(std::move(ptr.spProxy)) {}

  SafePtr() {}
  SafePtr(nullptr_t) {}

  explicit SafePtr(T *pInstance) : spProxy((internal::SafeProxy<T>*)internal::GetSafePtr(pInstance)) {}
  explicit SafePtr(const SharedPtr<T> &spInstance) : spProxy((internal::SafeProxy<T>*)internal::GetSafePtr(spInstance.ptr())) {}
  explicit SafePtr(const UniquePtr<T> &upInstance) : spProxy((internal::SafeProxy<T>*)internal::GetSafePtr(upInstance.ptr())) {}

  SafePtr& operator=(const SafePtr<T> &ptr)
  {
    spProxy = ptr.spProxy;
    return *this;
  }
  SafePtr& operator=(SafePtr<T> &&ptr)
  {
    spProxy = std::move(ptr.spProxy);
    return *this;
  }
  template<typename U>
  SafePtr& operator=(const SafePtr<U> &ptr)
  {
    spProxy = ptr.spProxy;
    return *this;
  }
  template<typename U>
  SafePtr& operator=(SafePtr<U> &&ptr)
  {
    spProxy.operator=(std::move(ptr.spProxy));
    return *this;
  }

  epforceinline explicit operator bool() const { return spProxy.count() > 0 && spProxy->pInstance; }

  epforceinline T& operator*() const { return *(T*)spProxy->pInstance; }
  epforceinline T* operator->() const { return (T*)spProxy->pInstance; }
  epforceinline T* ptr() const { return spProxy ? (T*)spProxy->pInstance : (T*)nullptr; }

  static_assert(std::is_base_of<Safe, T>::value, "T must inherit from Safe");

private:
  SharedPtr<internal::SafeProxy<T>> spProxy;
};


namespace internal {

template <typename T>
class SafeProxy : public RefCounted
{
public:
  SafeProxy(T *pInstance) : pInstance(pInstance) {}
  ~SafeProxy()
  {
    if (pInstance)
      internal::NullifySafePtr(pInstance);
  }
  T* pInstance;
};

} // namespace internal


inline Safe::~Safe()
{
  internal::NullifySafePtr(this);
}


// cast functions
template <class T, class U>
SafePtr<T> safe_pointer_cast(const SafePtr<U> &ptr);

// comparaison operators
template <class T, class U> inline bool operator==(const SafePtr<T> &l, const SafePtr<U> &r) { return l.ptr() == r.ptr(); }
template <class T, class U> inline bool operator!=(const SafePtr<T> &l, const SafePtr<U> &r) { return l.ptr() != r.ptr(); }
template <class T, class U> inline bool operator<=(const SafePtr<T> &l, const SafePtr<U> &r) { return l.ptr() <= r.ptr(); }
template <class T, class U> inline bool operator<(const SafePtr<T> &l, const SafePtr<U> &r) { return l.ptr() < r.ptr(); }
template <class T, class U> inline bool operator>=(const SafePtr<T> &l, const SafePtr<U> &r) { return l.ptr() >= r.ptr(); }
template <class T, class U> inline bool operator>(const SafePtr<T> &l, const SafePtr<U> &r) { return l.ptr() > r.ptr(); }
template <class T> inline bool operator==(const SafePtr<T> &l, nullptr_t) { return l.ptr() == nullptr; }
template <class T> inline bool operator!=(const SafePtr<T> &l, nullptr_t) { return l.ptr() != nullptr; }
template <class T> inline bool operator==(nullptr_t, const SafePtr<T> &r) { return nullptr == r.ptr(); }
template <class T> inline bool operator!=(nullptr_t, const SafePtr<T> &r) { return nullptr != r.ptr(); }

} // namespace ep

#include "ep/cpp/internal/safeptr_inl.h"

#endif // _EPSAFEPTR_HPP
