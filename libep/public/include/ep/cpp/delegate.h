#pragma once
#if !defined(_EPDELEGATE_HPP)
#define _EPDELEGATE_HPP

#include "ep/cpp/sharedptr.h"
#include "ep/cpp/internal/fastdelegate.h"

using fastdelegate::FastDelegate;
typedef fastdelegate::DelegateMemento FastDelegateMemento;

namespace ep {

namespace internal {
  // helper to get details from functions
  template<typename T>
  struct function_traits;

  template<typename T, typename Del, typename R, typename... Args>
  using enable_if_valid_functor = 
    typename std::enable_if<
      std::is_same<R(Args...), typename internal::function_traits<decltype(&std::remove_reference<T>::type::operator())>::function_signature>::value  // check has operator() and signature is the same
      && !std::is_convertible<T, R(*)(Args...)>::value  // check is it not implicitly convertable to function pointer (lambda without capture)
      && !std::is_same<typename std::remove_reference<T>::type, Del>::value // check is it not delegate itself
    >::type;
}

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
#if !defined(EP_WINDOWS)
    EPASSERT(u.ptrs[2] == nullptr, "Unexpected: method has data in the second size_t!");
#endif
  }
  template <typename T>
  epforceinline MethodPointer(R(T::*method)(Args... args) const) // TODO: accepts const methods, but const is not preserved!! fixme!
  {
    DelegateUnion u;
    u.d = FastDelegateType((T*)nullptr, method);
    ptr = u.ptrs[1];
#if !defined(EP_WINDOWS)
    EPASSERT(u.ptrs[2] == nullptr, "Unexpected: method has data in the second size_t!");
#endif
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
    DelegateUnion() : d() {}
    FastDelegateType d;
#if defined(EP_WINDOWS)
    static_assert(sizeof(FastDelegateType) == sizeof(void*)*2, "Unexpected FastDelegate size!");
    void *ptrs[2];
#else
    static_assert(sizeof(FastDelegateType) == sizeof(void*)*3, "Unexpected FastDelegate size!");
    void *ptrs[3];
#endif
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
struct Delegate<R(Args...) const>
{
  static_assert(sizeof(R) == 0, "Delegate signatures should not be 'const'");
};
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
  template <typename T, internal::enable_if_valid_functor<T, Delegate<R(Args...)>, R, Args...>* = nullptr>
  Delegate(T &&lambda);

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

  bool operator==(const Delegate &val)
  {
    return this->m == val.m;
  }

  bool operator!=(const Delegate &val)
  {
    return this->m != val.m;
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

#include "ep/cpp/internal/delegate_inl.h"

#endif // _EPDELEGATE_HPP
