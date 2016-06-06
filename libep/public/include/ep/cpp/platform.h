#pragma once
#if !defined(_EP_PLATFORM_HPP)
#define _EP_PLATFORM_HPP

#include "ep/c/platform.h"
#include <new>

#include "ep/cpp/error.h"
#include "ep/cpp/string.h"

template<typename ...Args>
inline void epDebugFormat(ep::String format, Args... args)
{
  ep::MutableString64 t(ep::Format, format, args...);
  epDebugWrite(t.ptr);
}

#if EPASSERT_ON

inline void epAssertFailed(ep::String condition, ep::String message, ep::String file, int line)
{
  epAssertFailed((epString&)condition, (epString&)message, (epString&)file, line);
}

// C++ assert code uses our fancy variadic format() function
namespace ep {
namespace internal {
  extern MutableString256 assertBuffer;
  inline void epAssertTemplate(String condition, String file, int line)
  {
    epAssertFailed(condition, nullptr, file, line);
  }
  template<typename ...Args>
  inline void epAssertTemplate(String condition, String file, int line, String format, Args... args)
  {
    assertBuffer.format(format, args...);
    epAssertFailed(condition, assertBuffer, file, line);
  }
} // namespace internal
} // namespace ep

# if defined(EPASSERT)
#   undef EPASSERT
# endif
# if defined(IF_EPASSERT)
#   undef IF_EPASSERT
# endif
# define EPASSERT(condition, ...) do { if (!(condition)) { ::ep::internal::epAssertTemplate(#condition, __FILE__, __LINE__, __VA_ARGS__); DebugBreak(); } } while (0)
# define IF_EPASSERT(x) x

# if defined(EPRELASSERT)
#   undef EPRELASSERT
# endif
# if defined(IF_EPRELASSERT)
#   undef IF_EPRELASSERT
# endif
# if EPRELASSERT_ON
#   define EPRELASSERT(condition, ...) do { if (!(condition)) { ::ep::internal::epAssertTemplate(#condition, __FILE__, __LINE__, __VA_ARGS__); DebugBreak(); } } while (0)
#   define IF_EPRELASSERT(x) x
# else
#   define EPRELASSERT(condition, ...) do {} while(0) // TODO: Make platform-specific __assume(condition)
#   define IF_EPRELASSERT(x)
# endif

#endif // EPASSERT_ON

#if __EP_MEMORY_DEBUG__ && defined(EP_WINDOWS)
# if defined(realloc)
#   undef realloc
# endif // defined(realloc)
#else // __EP_MEMORY_DEBUG__ && defined(EP_WINDOWS)
#endif  // __EP_MEMORY_DEBUG__ && defined(EP_WINDOWS)

#define epConstruct ::new

#if __EP_MEMORY_DEBUG__
# define epNew(type, ...) _epNew<type>(__FILE__, __LINE__, ##__VA_ARGS__)

template <typename Type, typename... Args>
inline Type* _epNew(const char *pFile, int line, Args&&... args)
{
  void *pMem = _epAlloc(sizeof(Type), epAF_None, pFile, line);
  epscope(fail) { _epFree(pMem); };
  return ::new(pMem) Type(std::forward<Args>(args)...);
}
#else
# define epNew(type, ...) _epNew<type>(__VA_ARGS__)

template <typename Type, typename... Args>
inline Type* _epNew(Args&&... args)
{
  void *pMem = _epAlloc(sizeof(Type), epAF_None);
  epscope(fail) { _epFree(pMem); };
  return ::new(pMem) Type(std::forward<Args>(args)...);
}
#endif

template <typename T>
void _epDelete(T *pMemory)
{
  if (pMemory)
  {
    pMemory->~T();
    _epFree((void*)(pMemory));
  }
}

#define epDelete(pMem) _epDelete(pMem)

#include "ep/cpp/internal/slice_inl.h"
#include "ep/cpp/internal/string_inl.h"

#endif // _EP_PLATFORM_HPP
