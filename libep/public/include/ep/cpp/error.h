#pragma once
#ifndef EPERROR_HPP
#define EPERROR_HPP

#include "ep/c/error.h"
#include "ep/cpp/string.h"

#include <utility>


#define PushError(error, message, ...) ep::_PushError(error, ep::MutableString<0>(ep::Format, ep::String(message), ##__VA_ARGS__), __PRETTY_FUNCTION__, __FILE__, __LINE__)


#ifdef EPERROR
# undef EPERROR
#endif
#define EPERROR(error, message, ...) (ep::LogError(ep::String(message), ##__VA_ARGS__), PushError(error, message, ##__VA_ARGS__))


#define EPTHROW(error, message, ...) throw ep::EPException(PushError(error, message, ##__VA_ARGS__))
#define EPTHROW_ERROR(error, message, ...) throw ep::EPException(EPERROR(error, message, ##__VA_ARGS__))
#define EPTHROW_WARN(error, level, message, ...) throw ep::EPException((ep::LogWarning(level, ep::String(message), ##__VA_ARGS__), PushError(error, message, ##__VA_ARGS__)))

#define EPTHROW_IF(condition, error, message, ...) { if(condition) { EPTHROW_ERROR(error, message, ##__VA_ARGS__); } }
#define EPTHROW_IF_NULL(condition, error, message, ...) { if((condition) == nullptr) { EPTHROW_ERROR(error, message, ##__VA_ARGS__); } }
#define EPTHROW_RESULT(error, message, ...) { epResult r = (error); if(r != epR_Success) { EPTHROW_ERROR(r, message, ##__VA_ARGS__); } }

#define EPASSERT_THROW(condition, error, message, ...) { if(!(condition)) { EPTHROW_ERROR(error, message, ##__VA_ARGS__); } }


namespace ep {

namespace internal {
  void Log(int type, int level, String text);
}

template<typename ...Args> inline void LogError(String text, Args... args);
template<typename ...Args> inline void LogWarning(int level, String text, Args... args);
template<typename ...Args> inline void LogDebug(int level, String text, Args... args);
template<typename ...Args> inline void LogInfo(int level, String text, Args... args);
template<typename ...Args> inline void LogScript(String text, Args... args);
template<typename ...Args> inline void LogTrace(String text, Args... args);

struct ErrorState
{
  epResult error;
  SharedString message;

  const char *function;
  const char *file;
  int line;

  ErrorState *pPrior;
};

ErrorState* _PushError(epResult error, const SharedString &message, const char *function, const char *file, int line);
void PopError();

void PopErrorToLevel(size_t level);

size_t ErrorLevel();
ErrorState* GetError();
SharedString GetErrorMessage();
void ClearError();

SharedString DumpError();


class EPException : public std::exception
{
public:
  EPException(ErrorState *pError);
  ~EPException();
  const char* what() const noexcept override;

  ErrorState *pError;
};


// implements ScopeGuard; a helper which calls a function on scope exit
template <typename Func>
class ScopeGuard
{
public:
  explicit ScopeGuard(const Func& fn)
    : function(fn), dismissed(false)
  {}
  explicit ScopeGuard(Func&& fn)
    : function(std::move(fn)), dismissed(false)
  {}
  ScopeGuard(ScopeGuard&& other)
    : function(std::move(other.function)), dismissed(other.dismissed)
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
