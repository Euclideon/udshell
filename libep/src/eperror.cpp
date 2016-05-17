#include "ep/cpp/platform.h"
#include "ep/cpp/error.h"
#include "ep/cpp/math.h"
#include "ep/cpp/kernel.h"

namespace ep {
namespace internal {

void Log(int type, int level, String text)
{
  Kernel *pK = Kernel::GetInstance();
  if (pK)
    pK->Log(type, level, text);
  else
    epDebugWrite(text.toStringz());
}

static inline ErrorSystem *GetErrorSystem()
{
  return (ErrorSystem*)s_pInstance->pErrorSystem;
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
  using namespace internal;
  ErrorSystem *pErrorSystem = GetErrorSystem();
  ErrorState *pErrorStack = pErrorSystem->errorStack;
  size_t &errorDepth = pErrorSystem->errorDepth;

  pErrorStack[errorDepth].error = error;
  epConstruct(&pErrorStack[errorDepth].message) SharedString(message);
  pErrorStack[errorDepth].function = function;
  pErrorStack[errorDepth].file = file;
  pErrorStack[errorDepth].line = line;
  pErrorStack[errorDepth].pPrior = errorDepth > 0 ? &pErrorStack[errorDepth-1] : nullptr;

  ErrorState *pState = &pErrorStack[errorDepth];
  // If the error state will exceeded the stack size just overwrite the last entry.
  // This is necessary to handle the case where script implements a loop that errors every iteration
  // and so may exceed the stack.
  errorDepth = ep::Min(ErrorSystem::ErrorStackSize - 1, ++errorDepth);
  return pState;
}
size_t ErrorLevel()
{
  return ep::internal::GetErrorSystem()->errorDepth;
}
ErrorState* GetError()
{
  using namespace internal;
  ErrorSystem *pErrorSystem = GetErrorSystem();
  return pErrorSystem->errorDepth ? &pErrorSystem->errorStack[pErrorSystem->errorDepth -1] : nullptr;
}

SharedString GetErrorMessage()
{
  ErrorState *pE = GetError();
  if (pE)
    return pE->message;

  return nullptr;
}

void ClearError()
{
  using namespace internal;
  ErrorSystem *pErrorSystem = GetErrorSystem();
  size_t &errorDepth = pErrorSystem->errorDepth;
  while (errorDepth)
    pErrorSystem->errorStack[--errorDepth].Clear();
}

void PopError()
{
  using namespace internal;
  ErrorSystem *pErrorSystem = GetErrorSystem();
  size_t &errorDepth = pErrorSystem->errorDepth;
  if (errorDepth)
    pErrorSystem ->errorStack[--errorDepth].Clear();
}

void PopErrorToLevel(size_t level)
{
  while (ep::internal::GetErrorSystem()->errorDepth > level)
    PopError();
}

SharedString DumpError()
{
  using namespace internal;
  ErrorSystem *pErrorSystem = GetErrorSystem();
  size_t depth = pErrorSystem->errorDepth;

  MutableString<0> s(Reserve, depth * 96);
  s = "Errors occurred!\n";

  while (depth-- > 0)
  {
    ErrorState &e = pErrorSystem->errorStack[depth];
    s.append(e.file, "(", e.line, "): Error ", (int)e.error, ": ", e.message);
  }

  return std::move(s);
}

} // namespace ep

// C bindings...
extern "C" {

epErrorState* epPushError(epResult error, epString message, const char *function, const char *file, int line)
{
  return (epErrorState*)ep::_PushError(error, ep::String(message), function, file, line);
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
  epConstruct(&r) ep::SharedString(ep::DumpError());
  return r;
}

} // extern "C"
