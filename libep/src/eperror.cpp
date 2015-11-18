#include "ep/cpp/platform.h"
#include "ep/cpp/error.h"

namespace ep {
namespace internal {

thread_local epErrorState s_errorStack[256];
thread_local size_t s_errorDepth = 0;

}
}

#if defined(__cplusplus)
extern "C" {
#endif

void epPushError(epResult error, epString message, epString file, int line)
{
  ep::internal::s_errorStack[ep::internal::s_errorDepth].error = error;
  ep::internal::s_errorStack[ep::internal::s_errorDepth].message = message;
  ep::internal::s_errorStack[ep::internal::s_errorDepth].file = file;
  ep::internal::s_errorStack[ep::internal::s_errorDepth].line = line;
  ep::internal::s_errorStack[ep::internal::s_errorDepth].pPredicate = ep::internal::s_errorDepth > 0 ? &ep::internal::s_errorStack[ep::internal::s_errorDepth-1] : nullptr;
}

epErrorState* epGetError()
{
  return ep::internal::s_errorDepth ? &ep::internal::s_errorStack[ep::internal::s_errorDepth-1] : nullptr;
}

void epClearError()
{
  ep::internal::s_errorDepth = 0;
}

epSharedString epDumpError()
{
  size_t depth = ep::internal::s_errorDepth;

  MutableString<0> s(Reserve, depth * 96);
  s = "Errors occurred!\n";

  while (depth-- > 0)
  {
    epErrorState &e = ep::internal::s_errorStack[depth];
    s.append((String)e.file, "(", e.line, "): Error ", (int)e.error, ": ", (String)e.message);
  }

  epSharedString r;
  new(&r) SharedString(s);
  return r;
}

#if defined(__cplusplus)
}
#endif
