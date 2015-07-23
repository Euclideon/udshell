#pragma once
#if !defined(_UDDELEGATE_H)
#define _UDDELEGATE_H

#include "3rdparty/FastDelegate.h"
#include "udSharedPtr.h"

using fastdelegate::FastDelegate;
typedef fastdelegate::DelegateMemento FastDelegateMemento;


class udDelegateMemento : public udRefCounted
{
protected:
  template<typename Signature>
  friend class udDelegate;
  template<typename T>
  friend class udSharedPtr;

  udDelegateMemento() {}
  udDelegateMemento(FastDelegateMemento m) : m(m) {}

  FastDelegateMemento m;
};
typedef udSharedPtr<udDelegateMemento> udDelegateMementoRef;


// facilitate using the function template syntax
template<typename Signature>
class udDelegate;

template<typename R, typename... Args>
class udDelegate<R(Args...)>
{
public:
  udDelegate() {}
  udDelegate(udDelegate<R(Args...)> &&rval)
  {
    // blind-copy the pointer
    (void*&)m = (void*&)rval.m;
    // inhibit the rval destructor
    (void*&)rval.m = nullptr;
  }
  udDelegate(const udDelegate<R(Args...)> &d)   : m(d.m) {}

  udDelegate(udSharedPtr<udDelegateMemento> m)  : m(m) {}

  udDelegate(FastDelegate<R(Args...)> d)        : m(udDelegateMementoRef::create(d.GetMemento())) {}
  template <class X, class Y>
  udDelegate(Y *i, R(X::*f)(Args...))           : udDelegate(Delegate(i, f)) {}
  template <class X, class Y>
  udDelegate(Y *i, R(X::*f)(Args...) const)     : udDelegate(Delegate(i, f)) {}
  udDelegate(R(*f)(Args...))                    : udDelegate(Delegate(f)) {}

  UDFORCE_INLINE R operator()(Args... args) const
  {
    Delegate d;
    d.SetMemento(m->m);
    return d(args...);
  }

  void SetMemento(udDelegateMementoRef m) { this->m = m; }
  udDelegateMementoRef GetMemento() const { return m; }

protected:
  typedef FastDelegate<R(Args...)> Delegate;

  udDelegateMementoRef m = nullptr;
};


#include "udDelegate.inl"

#endif // _UDDELEGATE_H
