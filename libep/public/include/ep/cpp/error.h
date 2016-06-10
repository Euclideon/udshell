#pragma once
#ifndef EPERROR_HPP
#define EPERROR_HPP

#include "ep/cpp/string.h"

#include <utility>

#define ConstructException(error, message, ...) ep::EPException(error, ep::MutableString<0>(ep::Format, ep::String(message), ##__VA_ARGS__), __PRETTY_FUNCTION__, __FILE__, __LINE__)
#define AllocError(error, message, ...) ep::internal::_AllocError(error, ep::MutableString<0>(ep::Format, ep::String(message), ##__VA_ARGS__), __PRETTY_FUNCTION__, __FILE__, __LINE__, nullptr)

#define EPTHROW(error, message, ...) throw ConstructException(error, message, ##__VA_ARGS__)
#define EPTHROW_ERROR(error, message, ...) throw (ep::LogError(ep::String(message), ##__VA_ARGS__), ConstructException(error, message, ##__VA_ARGS__))
#define EPTHROW_WARN(error, level, message, ...) throw (ep::LogWarning(level, ep::String(message), ##__VA_ARGS__), ConstructException(error, message, ##__VA_ARGS__))

#define EPTHROW_IF(condition, error, message, ...) { if(condition) { EPTHROW_ERROR(error, message, ##__VA_ARGS__); } }
#define EPTHROW_IF_NULL(condition, error, message, ...) { if((condition) == nullptr) { EPTHROW_ERROR(error, message, ##__VA_ARGS__); } }
#define EPTHROW_RESULT(error, message, ...) { Result r = (error); if(r != Result::Success) { EPTHROW_ERROR(r, message, ##__VA_ARGS__); } }

#if EP_DEBUG
#define EPASSERT_THROW(condition, error, message, ...) { if(!(condition)) { DebugBreak(); EPTHROW_ERROR(error, message, ##__VA_ARGS__); } }
#else
#define EPASSERT_THROW(condition, error, message, ...) { if(!(condition)) { EPTHROW_ERROR(error, message, ##__VA_ARGS__); } }
#endif


// TODO: deprecate and remove these...
#define EP_ERROR_BREAK_ON_ERROR 0  // Set to 1 to have the debugger break on error
#define EP_ERROR_IF(x, code)      do { if (x) { result = code; if (EP_ERROR_BREAK_ON_ERROR && code) { __debugbreak(); } goto epilogue; }                  } while(0)
#define EP_ERROR_NULL(ptr, code)  do { if (ptr == nullptr) { result = code; if (EP_ERROR_BREAK_ON_ERROR) { __debugbreak(); } goto epilogue; }             } while(0)
#define EP_ERROR_CHECK(funcCall)  do { result = funcCall; if (result != epR_Success) { if (EP_ERROR_BREAK_ON_ERROR) { __debugbreak(); } goto epilogue; }  } while(0)
#define EP_ERROR_HANDLE()         do { if (result != epR_Success) { if (EP_ERROR_BREAK_ON_ERROR) { __debugbreak(); } goto epilogue; }                     } while(0)


namespace ep {

enum class Result
{
  Success,
  Failure,

  CppException,
  InvalidCall,
  InvalidArgument,
  InvalidType,
  OutOfBounds,
  BadCast,
  AllocFailure,
  ResourceInUse,
  AlreadyExists,

  File_OpenFailure,

  ScriptException,
};

template<typename ...Args> inline void LogError(String text, Args... args);
template<typename ...Args> inline void LogWarning(int level, String text, Args... args);
template<typename ...Args> inline void LogDebug(int level, String text, Args... args);
template<typename ...Args> inline void LogInfo(int level, String text, Args... args);
template<typename ...Args> inline void LogScript(String text, Args... args);
template<typename ...Args> inline void LogTrace(String text, Args... args);

struct ErrorState
{
  void Clear()
  {
    message.~SharedString();
    message.ptr = nullptr;
    message.length = 0;
  }

  Result error;
  SharedString message;

  const char *function;
  const char *file;
  int line;

  ErrorState *pPrior = nullptr;
};


class EPException : public std::exception
{
public:
  EPException(Result error, const SharedString &message, const char *function, const char *file, int line, ErrorState *pPrior = nullptr);
  EPException(const EPException &e) : pError(e.pError) {}
  EPException(EPException &&e) : pError(e.pError) { e.pError = nullptr; }
  EPException(ErrorState *pError) : pError(pError) {}
  ~EPException();

  const char* what() const noexcept override { return pError->message.ptr; }

  ErrorState *claim() noexcept
  {
    ErrorState *pR = pError;
    pError = nullptr;
    return pR;
  }

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


SharedString DumpError(ErrorState *pError);


// helper to make a scope guard
template <typename Func>
ScopeGuard<typename std::decay<Func>::type> makeGuard(Func&& fn)
{
  return ScopeGuard<typename std::decay<Func>::type>(std::forward<Func>(fn));
}

namespace internal {
  ErrorState* _AllocError(Result error, const SharedString &message, const char *function, const char *file, int line, ErrorState *pParent);
  void Log(int type, int level, String text);
} // namespace internal

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
