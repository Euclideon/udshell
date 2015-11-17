#pragma once
#if !defined(_EPDELEGATE_HPP)
#define _EPDELEGATE_HPP

#include "ep/cpp/sharedptr.h"
#include "ep/epfastdelegate.h"

using fastdelegate::FastDelegate;
typedef fastdelegate::DelegateMemento FastDelegateMemento;

namespace ep {

class DelegateMemento : public RefCounted
{
public:
  epforceinline FastDelegateMemento GetFastDelegate() const { return m; }

protected:
  template<typename Signature>
  friend class Delegate;
  template<typename T>
  friend class SharedPtr;

  DelegateMemento() {}
  DelegateMemento(FastDelegateMemento m) : m(m) {}

  FastDelegateMemento m;
};
typedef SharedPtr<DelegateMemento> DelegateMementoRef;


// facilitate using the function template syntax
template<typename Signature>
class Delegate;

template<typename R, typename... Args>
class Delegate<R(Args...)>
{
public:
  Delegate() {}
  Delegate(nullptr_t) {}
  Delegate(Delegate<R(Args...)> &&rval)
  {
    // blind-copy the pointer
    (void*&)m = (void*&)rval.m;
    // inhibit the rval destructor
    (void*&)rval.m = nullptr;
  }
  Delegate(const Delegate<R(Args...)> &d) : m(d.m) {}

  Delegate(SharedPtr<DelegateMemento> m) : m(m) {}

  Delegate(FastDelegate<R(Args...)> d) : m(DelegateMementoRef::create(d.GetMemento())) {}
  template <class X, class Y>
  Delegate(Y *i, R(X::*f)(Args...)) : Delegate(FD(i, f)) {}
  template <class X, class Y>
  Delegate(Y *i, R(X::*f)(Args...) const) : Delegate(FD(i, f)) {}
  Delegate(R(*f)(Args...)) : Delegate(FD(f)) {}

  explicit operator bool() { return m ? !m->m.empty() : false; }

  Delegate& operator=(const Delegate &d)
  {
    if (this != &d)
    {
      this->~Delegate();
      new (this) Delegate(d);
    }
    return *this;
  }

  Delegate& operator=(Delegate &&rval)
  {
    if (this != &rval)
    {
      (void*&)m = (void*&)rval.m;
      (void*&)rval.m = nullptr;
    }
    return *this;
  }

  epforceinline R operator()(Args... args) const
  {
    FD d;
    d.SetMemento(m->m);
    return d(args...);
  }

  void SetMemento(DelegateMementoRef _m) { this->m = _m; }
  DelegateMementoRef GetMemento() const { return m; }

protected:
  typedef FastDelegate<R(Args...)> FD;

  DelegateMementoRef m = nullptr;
};

template<typename R, typename... Args>
inline ptrdiff_t epStringify(Slice<char> epUnusedParam(buffer), String epUnusedParam(format), Delegate<R(Args...)> epUnusedParam(d), const epVarArg *epUnusedParam(pArgs))
{
  EPASSERT(false, "TODO! Write me!");
  return 0;
}

} // namespace ep

#endif // _EPDELEGATE_HPP
