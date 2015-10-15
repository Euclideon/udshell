#pragma once
#if !defined(_EP_PLATFORM_HPP)
#define _EP_PLATFORM_HPP

#include "ep/c/platform.h"

namespace ep {}
using namespace ep;

#include "ep/cpp/error.h"
#include "ep/cpp/string.h"

template<typename ...Args>
inline void epDebugFormat(String format, Args... args)
{
  MutableString64 t.format(format, args...);
  epDebugWrite(t.ptr);
}

#if EPASSERT_ON

inline void epAssertFailed(String condition, String message, String file, int line)
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
#   define EPRELASSERT(condition, ...) // TODO: Make platform-specific __assume(condition)
#   define IF_EPRELASSERT(x)
# endif

#endif // EPASSERT_ON


#include "ep/cpp/internal/slice_inl.h"
#include "ep/cpp/internal/string_inl.h"

#endif // _EP_PLATFORM_HPP
