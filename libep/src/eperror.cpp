#include "ep/cpp/platform.h"
#include "ep/cpp/error.h"

#include "ep/cpp/kernel.h"

namespace ep {
namespace internal {

thread_local ErrorState s_errorStack[256];
thread_local size_t s_errorDepth = 0;

void Log(int type, int level, String text)
{
  Kernel *pK = Kernel::GetInstance();
  if (pK)
    pK->Log(type, level, text);
  else
    epDebugWrite(text.toStringz());
}

} // namespace internal

EPException::EPException(ErrorState *_pError)
  : pError(_pError)
{}
EPException::~EPException()
{}
const char* EPException::what() const noexcept
{
  return pError->message.ptr;
}

ErrorState* _PushError(epResult error, const SharedString &message, const char *function, const char *file, int line)
{
  ep::internal::s_errorStack[ep::internal::s_errorDepth].error = error;
  new(&ep::internal::s_errorStack[ep::internal::s_errorDepth].message) SharedString(message);
  ep::internal::s_errorStack[ep::internal::s_errorDepth].function = function;
  ep::internal::s_errorStack[ep::internal::s_errorDepth].file = file;
  ep::internal::s_errorStack[ep::internal::s_errorDepth].line = line;
  ep::internal::s_errorStack[ep::internal::s_errorDepth].pPrior = ep::internal::s_errorDepth > 0 ? &ep::internal::s_errorStack[ep::internal::s_errorDepth-1] : nullptr;
  return &ep::internal::s_errorStack[ep::internal::s_errorDepth++];
}

size_t ErrorLevel()
{
  return ep::internal::s_errorDepth;
}
ErrorState* GetError()
{
  return ep::internal::s_errorDepth ? &ep::internal::s_errorStack[ep::internal::s_errorDepth-1] : nullptr;
}
void ClearError()
{
  while (ep::internal::s_errorDepth)
  {
    ep::internal::s_errorStack[ep::internal::s_errorDepth-1].~ErrorState();
    --ep::internal::s_errorDepth;
  }
}

SharedString DumpError()
{
  size_t depth = ep::internal::s_errorDepth;

  MutableString<0> s(Reserve, depth * 96);
  s = "Errors occurred!\n";

  while (depth-- > 0)
  {
    ErrorState &e = ep::internal::s_errorStack[depth];
    s.append(e.file, "(", e.line, "): Error ", (int)e.error, ": ", e.message);
  }

  return std::move(s);
}

} // namespace ep

// C bindings...
extern "C" {

epErrorState* epPushError(epResult error, epString message, const char *function, const char *file, int line)
{
  return (epErrorState*)ep::_PushError(error, String(message), function, file, line);
}

size_t epErrorLevel()
{
  return ep::ErrorLevel();
}
epErrorState* epGetError()
{
  return (epErrorState*)ep::GetError();
}
void epClearError()
{
  ep::ClearError();
}

epSharedString epDumpError()
{
  epSharedString r;
  new(&r) SharedString(ep::DumpError());
  return r;
}

} // extern "C"
