#pragma once
#if !defined(_EPDELEGATE_HPP)
#define _EPDELEGATE_HPP

#include "ep/cpp/sharedptr.h"
#include "ep/cpp/internal/fastdelegate.h"

using fastdelegate::FastDelegate;
typedef fastdelegate::DelegateMemento FastDelegateMemento;

namespace ep {

/// \cond
template<typename Signature>
struct MethodPointer;
template<typename Signature>
struct Delegate;
/// \endcond

// helper to handle method pointers
template <typename R, typename... Args>
struct MethodPointer<R(Args...)>
{
  typedef FastDelegate<R(Args...)> FastDelegateType;

  MethodPointer() : ptr(nullptr) {}
  MethodPointer(nullptr_t) : ptr(nullptr) {}

  epforceinline MethodPointer(const MethodPointer &rh) : ptr(rh.ptr) {}

  template <typename T>
  epforceinline MethodPointer(R(T::*method)(Args... args))
  {
    DelegateUnion u;
    u.d = FastDelegateType((T*)nullptr, method);
    ptr = u.ptrs[1];
  }

  epforceinline FastDelegateType GetDelegate(void *pThis) const
  {
    DelegateUnion u;
    u.ptrs[0] = pThis;
    u.ptrs[1] = ptr;
    return u.d;
  }

  epforceinline R Call(void *pThis, Args... args) const
  {
    return GetDelegate(pThis)(args...);
  }

  epforceinline explicit operator bool() const { return ptr != nullptr; }

private:
  union DelegateUnion
  {
    DelegateUnion() {}
    FastDelegateType d;
    void *ptrs[2];
  };

  void *ptr;
};


class DelegateMemento : public RefCounted
{
public:
  epforceinline FastDelegateMemento GetFastDelegate() const { return m; }

protected:
  template<typename Signature>
  friend struct Delegate;
  template<typename T>
  friend struct SharedPtr;

  DelegateMemento() {}
  DelegateMemento(FastDelegateMemento m) : m(m) {}

  FastDelegateMemento m;
};
typedef SharedPtr<DelegateMemento> DelegateMementoRef;

// facilitate using the function template syntax
template<typename R, typename... Args>
struct Delegate<R(Args...)>
{
public:
  typedef FastDelegate<R(Args...)> FastDelegateType;

  Delegate() {}
  Delegate(nullptr_t) {}
  Delegate(Delegate<R(Args...)> &&rval) : m(std::move(rval.m)) {}
  Delegate(const Delegate<R(Args...)> &d) : m(d.m) {}

  Delegate(DelegateMementoRef m) : m(m) {}

  Delegate(FastDelegate<R(Args...)> d) : m(DelegateMementoRef::create(d.GetMemento())) {}
  template <class X, class Y>
  Delegate(Y *i, R(X::*f)(Args...)) : Delegate(FastDelegateType(i, f)) {}
  template <class X, class Y>
  Delegate(Y *i, R(X::*f)(Args...) const) : Delegate(FastDelegateType(i, f)) {}
  Delegate(R(*f)(Args...)) : Delegate(FastDelegateType(f)) {}

  explicit operator bool() const { return m ? !m->m.empty() : false; }

  Delegate& operator=(const Delegate &d)
  {
    if (this != &d)
    {
      this->~Delegate();
      epConstruct (this) Delegate(d);
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
    FastDelegateType d;
    d.SetMemento(m->m);
    return d(args...);
  }

  void SetMemento(DelegateMementoRef _m) { this->m = _m; }
  DelegateMementoRef GetMemento() const { return m; }

protected:
  friend struct Variant;

  DelegateMementoRef m = nullptr;
};

template<typename R, typename... Args>
inline ptrdiff_t epStringify(Slice<char> epUnusedParam(buffer), String epUnusedParam(format), Delegate<R(Args...)> epUnusedParam(d), const VarArg *epUnusedParam(pArgs))
{
  EPASSERT(false, "TODO! Write me!");
  return 0;
}

} // namespace ep

#endif // _EPDELEGATE_HPP
