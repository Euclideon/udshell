#pragma once
#ifndef EPERROR_HPP
#define EPERROR_HPP

#include "ep/c/error.h"
#include "ep/cpp/string.h"

#include <utility>

#ifdef EPERROR
# undef EPERROR
#endif

#define PushError(error, message) epPushError(error, String(message), String(__FILE__), __LINE__)

#define EPERROR(error, message) PushError(error, message)

#define EPTHROW(error, message) throw PushError(error, message)
#define EPTHROW_NULL(condition, error, message) if((condition) == nullptr) { throw PushError(error, message); }

namespace ep {

epErrorState* GetError();
void ClearError();

SharedString DumpError();

// implements ScopeGuard; a helper which calls a function on scope exit
template <typename Func>
class ScopeGuard
{
public:
  explicit ScopeGuard(const Func& fn)
    : dismissed(false), function(fn)
  {}
  explicit ScopeGuard(Func&& fn)
    : dismissed(false), function(std::move(fn))
  {}
  ScopeGuard(ScopeGuard&& other)
    : dismissed(other.dismissed), function(std::move(other.function))
  {
    other.dismissed = true; // inhibit the destructor of original object
  }

  ~ScopeGuard() noexcept
  {
    if (!dismissed)
      function();
  }

private:
  void* operator new(size_t) = delete; // can not dynamically allocate!

  Func function;
  bool dismissed;
};

// helper to make a scope guard
template <typename Func>
ScopeGuard<typename std::decay<Func>::type> makeGuard(Func&& fn)
{
  return ScopeGuard<typename std::decay<Func>::type>(std::forward<Func>(fn));
}

} // namespace ep


// macros for SCOPE_EXIT/FAIL/SUCCESS
#define SCOPE_EXIT \
  auto ANONYMOUS_VAR(SCOPE_EXIT_STATE) = ::ep::internal::ScopeGuardOnExit() + [&]() epnothrow

#define SCOPE_FAIL \
  auto ANONYMOUS_VAR(SCOPE_FAIL_STATE) = ::ep::internal::ScopeGuardOnFail() + [&]() epnothrow

#define SCOPE_SUCCESS \
  auto ANONYMOUS_VAR(SCOPE_SUCCESS_STATE) = ::ep::internal::ScopeGuardOnSuccess() + [&]()


// scope statement style macro
#define epscope(type) SCOPE_##type

// adapters
#define SCOPE_exit SCOPE_EXIT
#define SCOPE_fail SCOPE_FAIL
#define SCOPE_success SCOPE_SUCCESS


#include "ep/cpp/internal/error_inl.h"

#endif // EPERROR_HPP
