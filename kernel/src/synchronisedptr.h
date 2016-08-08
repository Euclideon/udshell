#pragma once
#ifndef EPSYNCHRONISEDPTR_H
#define EPSYNCHRONISEDPTR_H

#include "ep/cpp/sharedptr.h"

namespace ep {

class Kernel;

namespace internal {

void destroyOnMainThread(Kernel *pKernel, RefCounted *pInstance);

} // namespace internal


// synchronised pointers that can be passed between threads, destruction is deferred to main thread
template<class T>
struct SynchronisedPtr
{
public:
  static_assert(std::is_base_of<RefCounted, T>::value, "T must derive from RefCounted");

  SynchronisedPtr() {}
  template <class U>
  explicit SynchronisedPtr(const SharedPtr<U> &p, Kernel *pKernel)
    : pInstance((T*)p.acquire(p.get()))
    , pKernel(pKernel)
  {
    EPASSERT(pKernel, "SynchronisedPtr must have a valid Kernel pointer!");
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
    if (pInstance)
      destroy();
  }

  SynchronisedPtr &operator=(const SynchronisedPtr &ptr)
  {
    if (this->pInstance == ptr.pInstance)
      return *this;

    if (pInstance)
      destroy();
    pInstance = ptr.pInstance;
    pKernel = ptr.pKernel;
    const_cast<SynchronisedPtr&>(ptr).pInstance = nullptr;
    const_cast<SynchronisedPtr&>(ptr).pKernel = nullptr;
    return *this;
  }

  void reset()
  {
    if (pInstance)
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
  T* get() const { return pInstance; }

private:
  template<typename U> friend struct SynchronisedPtr;

  void destroy()
  {
    internal::destroyOnMainThread(pKernel, pInstance);
    pInstance = nullptr;
  }

  T *pInstance = nullptr;
  Kernel *pKernel = nullptr;
};

} // namespace ep

#endif // EPSYNCHRONISEDPTR_H
