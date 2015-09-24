#pragma once
#if !defined(_EPDELEGATE_H)
#define _EPDELEGATE_H

#include "ep/epfastdelegate.h"
#include "ep/epsharedptr.h"

using fastdelegate::FastDelegate;
typedef fastdelegate::DelegateMemento FastDelegateMemento;

class epDelegateMemento : public epRefCounted
{
protected:
  template<typename Signature>
  friend class epDelegate;
  template<typename T>
  friend class epSharedPtr;

  epDelegateMemento() {}
  epDelegateMemento(FastDelegateMemento m) : m(m) {}

  FastDelegateMemento m;
};
typedef epSharedPtr<epDelegateMemento> epDelegateMementoRef;


// facilitate using the function template syntax
template<typename Signature>
class epDelegate;

template<typename R, typename... Args>
class epDelegate<R(Args...)>
{
public:
  epDelegate() {}
  epDelegate(nullptr_t) {}
  epDelegate(epDelegate<R(Args...)> &&rval)
  {
    // blind-copy the pointer
    (void*&)m = (void*&)rval.m;
    // inhibit the rval destructor
    (void*&)rval.m = nullptr;
  }
  epDelegate(const epDelegate<R(Args...)> &d)   : m(d.m) {}

  epDelegate(epSharedPtr<epDelegateMemento> m)  : m(m) {}

  epDelegate(FastDelegate<R(Args...)> d)        : m(epDelegateMementoRef::create(d.GetMemento())) {}
  template <class X, class Y>
  epDelegate(Y *i, R(X::*f)(Args...))           : epDelegate(Delegate(i, f)) {}
  template <class X, class Y>
  epDelegate(Y *i, R(X::*f)(Args...) const)     : epDelegate(Delegate(i, f)) {}
  epDelegate(R(*f)(Args...))                    : epDelegate(Delegate(f)) {}

  explicit operator bool () { return m ? !m->m.empty() : false; }

  epDelegate& operator=(const epDelegate &d)
  {
    if (this != &d)
    {
      this->~epDelegate();
      new (this) epDelegate(d);
    }
    return *this;
  }

  epDelegate& operator=(epDelegate &&rval)
  {
    if (this != &rval)
    {
      (void*&)m = (void*&)rval.m;
      (void*&)rval.m = nullptr;
    }
    return *this;
  }


  __forceinline R operator()(Args... args) const
  {
    Delegate d;
    d.SetMemento(m->m);
    return d(args...);
  }

  void SetMemento(epDelegateMementoRef m) { this->m = m; }
  epDelegateMementoRef GetMemento() const { return m; }

protected:
  typedef FastDelegate<R(Args...)> Delegate;

  epDelegateMementoRef m = nullptr;
};

template<typename R, typename... Args>
inline ptrdiff_t epStringify(epSlice<char> epUnusedParam(buffer), epString epUnusedParam(format), epDelegate<R(Args...)> epUnusedParam(d), const epVarArg *epUnusedParam(pArgs))
{
  EPASSERT(false, "TODO! Write me!");
  return 0;
}

#endif // _EPDELEGATE_H
