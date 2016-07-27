// HACK: make an anonymous variable name...
#ifndef ANONYMOUS_VAR
# define EP_CONCATENATE_IMPL(s1, s2) s1##s2
# define EP_CONCATENATE(s1, s2) EP_CONCATENATE_IMPL(s1, s2)
#ifdef __COUNTER__
# define ANONYMOUS_VAR(str) EP_CONCATENATE(str, __COUNTER__)
#else
# define ANONYMOUS_VAR(str) EP_CONCATENATE(str, __LINE__)
# endif
#endif


// HACK: machinery to get information from the runtime about exceptions in flight
#if defined(__GNUG__) || defined(__CLANG__)
# define EXCEPTION_COUNT_USE_CXA_GET_GLOBALS
  namespace __cxxabiv1 {
    // forward declaration (originally defined in unwind-cxx.h from from libstdc++)
    struct __cxa_eh_globals;
    // declared in cxxabi.h from libstdc++-v3
    extern "C" __cxa_eh_globals* __cxa_get_globals() noexcept;
  }
#elif defined(_MSC_VER) && (_MSC_VER >= 1900) // VS2015
# define EXCEPTION_COUNT_USE_CPP17
#elif defined(_MSC_VER) && (_MSC_VER >= 1400) // MSVC++ 8.0 or greater
# define EXCEPTION_COUNT_USE_GETPTD
  // forward declaration (originally defined in mtdll.h from MSVCRT)
  struct _tiddata;
  extern "C" _tiddata* _getptd(); // declared in mtdll.h from MSVCRT
#else
# error "Unsupported platform!"
#endif

namespace ep {
namespace internal {

struct ErrorSystem
{
  ErrorSystem();
  ~ErrorSystem();

  ErrorState *pError = nullptr;

  void *pErrorPool = nullptr;
};

inline int getUncaughtExceptionCount() noexcept
{
#if defined(EXCEPTION_COUNT_USE_CXA_GET_GLOBALS)
  // __cxa_get_globals returns a __cxa_eh_globals* (defined in unwind-cxx.h).
  // The offset below returns __cxa_eh_globals::uncaughtExceptions.
  return *(unsigned int*)((char*)__cxxabiv1::__cxa_get_globals() + sizeof(void*));
#elif defined(EXCEPTION_COUNT_USE_GETPTD)
  // _getptd() returns a _tiddata* (defined in mtdll.h).
  // The offset below returns _tiddata::_ProcessingThrow.
  return *(int*)((char*)_getptd() + sizeof(void*) * 28 + 0x4 * 8);
#elif defined(EXCEPTION_COUNT_USE_CPP17)
  return std::uncaught_exceptions();
#endif
}

template <typename Func, bool executeOnException>
class ScopeGuardForNewException
{
public:
  explicit ScopeGuardForNewException(const Func& fn)
    : function(fn), exceptionCount(getUncaughtExceptionCount())
  {}
  explicit ScopeGuardForNewException(Func&& fn)
    : function(std::move(fn)), exceptionCount(getUncaughtExceptionCount())
  {}
  ScopeGuardForNewException(ScopeGuardForNewException&& other)
    : function(std::move(other.function)), exceptionCount(other.exceptionCount)
  {}

  ~ScopeGuardForNewException() noexcept(executeOnException)
  {
    if (executeOnException == (getUncaughtExceptionCount() > exceptionCount))
      function();
  }

private:
  ScopeGuardForNewException(const ScopeGuardForNewException& other) = delete;
  void* operator new(size_t) = delete; // can not dynamically allocate!

  Func function;
  int exceptionCount;
};

// implementation for SCOPE_FAIL
enum class ScopeGuardOnFail {};
template <typename Func>
ScopeGuardForNewException<typename std::decay<Func>::type, true> operator +(ScopeGuardOnFail, Func&& fn)
{
  return ScopeGuardForNewException<typename std::decay<Func>::type, true>(std::forward<Func>(fn));
}

// implementation for SCOPE_SUCCESS
enum class ScopeGuardOnSuccess {};
template <typename Func>
ScopeGuardForNewException<typename std::decay<Func>::type, false> operator +(ScopeGuardOnSuccess, Func&& fn)
{
  return ScopeGuardForNewException<typename std::decay<Func>::type, false>(std::forward<Func>(fn));
}

// implementation for SCOPE_EXIT
enum class ScopeGuardOnExit {};
template <typename Func>
ScopeGuard<typename std::decay<Func>::type> operator +(ScopeGuardOnExit, Func&& fn)
{
  return ScopeGuard<typename std::decay<Func>::type>(std::forward<Func>(fn));
}

} // namespace internal

// global log functions
template<typename ...Args> inline void logError(String text, Args... args)
{
  if (sizeof...(Args) == 0)
    internal::log(1<<0, 2, text);
  else
  {
    MutableString128 tmp(Format, text, args...);
    internal::log(1<<0, 2, tmp);
  }
}
template<typename ...Args> inline void logWarning(int level, String text, Args... args)
{
  if (sizeof...(Args) == 0)
    internal::log(1<<1, level, text);
  else
  {
    MutableString128 tmp(Format, text, args...);
    internal::log(1<<1, level, tmp);
  }
}
template<typename ...Args> inline void logDebug(int level, String text, Args... args)
{
  if (sizeof...(Args) == 0)
    internal::log(1<<2, level, text);
  else
  {
    MutableString128 tmp(Format, text, args...);
    internal::log(1<<2, level, tmp);
  }
}
template<typename ...Args> inline void logInfo(int level, String text, Args... args)
{
  if (sizeof...(Args) == 0)
    internal::log(1<<3, level, text);
  else
  {
    MutableString128 tmp(Format, text, args...);
    internal::log(1<<3, level, tmp);
  }
}
template<typename ...Args> inline void logScript(String text, Args... args)
{
  if (sizeof...(Args) == 0)
    internal::log(1<<4, 2, text);
  else
  {
    MutableString128 tmp(Format, text, args...);
    internal::log(1<<4, 2, tmp);
  }
}
template<typename ...Args> inline void logTrace(String text, Args... args)
{
  if (sizeof...(Args) == 0)
    internal::log(1<<5, 2, text);
  else
  {
    MutableString128 tmp(Format, text, args...);
    internal::log(1<<5, 2, tmp);
  }
}

} // namespace ep
