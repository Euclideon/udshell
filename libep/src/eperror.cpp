#include "libep_internal.h"
#include "ep/cpp/platform.h"
#include "ep/cpp/error.h"
#include "ep/cpp/math.h"
#include "ep/cpp/kernel.h"
#include "ep/cpp/freelist.h"

namespace ep {
namespace internal {

void log(int type, int level, String text)
{
  Kernel *pK = Kernel::getInstance();
  if (pK)
    pK->log(type, level, text);
  else
    epDebugWrite(text.toStringz());
}

ErrorSystem::ErrorSystem()
{
  pErrorPool = epNew(FreeList<ErrorState>, 64);
}
ErrorSystem::~ErrorSystem()
{
  epDelete((FreeList<ErrorState>*)pErrorPool);
}

inline ErrorSystem& getErrorSystem()
{
  static ErrorSystem *pSystem = nullptr;
  if (!pSystem)
    pSystem = (ErrorSystem*)s_pInstance->pErrorSystem;
  return *pSystem;
}

inline FreeList<ErrorState>& getErrorPool()
{
  return *(FreeList<ErrorState>*)getErrorSystem().pErrorPool;
}

ErrorState* _allocError(Result error, const SharedString &message, const char *function, const char *file, int line, ErrorState *pParent)
{
  ErrorState *pError = internal::getErrorPool()._alloc();
  pError->error = error;
  epConstruct(&pError->message) SharedString(message);
  pError->function = function;
  pError->file = file;
  pError->line = line;
  pError->pPrior = pParent;
  return pError;
}

} // namespace internal

EPException::EPException(Result error, const SharedString &message, const char *function, const char *file, int line, ErrorState *pPrior)
  : pError(internal::_allocError(error, message, function, file, line, pPrior))
{}

EPException::~EPException()
{
  FreeList<ErrorState>& pool = internal::getErrorPool();
  while (pError)
  {
    ErrorState *pPrev = pError->pPrior;
    pError->clear();
    pool._free(pError);
    pError = pPrev;
  }
}


ErrorState* _pushError(Result error, const SharedString &message, const char *function, const char *file, int line)
{
  internal::ErrorSystem& errorSystem = internal::getErrorSystem();
  errorSystem.pError = internal::_allocError(error, message, function, file, line, errorSystem.pError);
  return errorSystem.pError;
}
ErrorState* getError()
{
  return internal::getErrorSystem().pError;
}

void clearError()
{
  internal::ErrorSystem& errorSystem = internal::getErrorSystem();
  FreeList<ErrorState>& pool = internal::getErrorPool();
  while (errorSystem.pError)
  {
    ErrorState *pPrev = errorSystem.pError->pPrior;
    errorSystem.pError->clear();
    pool._free(errorSystem.pError);
    errorSystem.pError = pPrev;
  }
}

SharedString dumpError(ErrorState *pError)
{
  MutableString<0> s(Reserve, 256);
  s = "Errors occurred!\n";

  while (pError)
  {
    s.append(pError->file, "(", pError->line, "): Error ", (int)pError->error, ": ", pError->message);
    pError = pError->pPrior;
  }

  return std::move(s);
}

} // namespace ep
