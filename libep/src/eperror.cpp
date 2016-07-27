#include "libep_internal.h"
#include "ep/cpp/platform.h"
#include "ep/cpp/error.h"
#include "ep/cpp/math.h"
#include "ep/cpp/kernel.h"
#include "ep/cpp/freelist.h"

namespace ep {
namespace internal {

void Log(int type, int level, String text)
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

inline ErrorSystem& GetErrorSystem()
{
  static ErrorSystem *pSystem = nullptr;
  if (!pSystem)
    pSystem = (ErrorSystem*)s_pInstance->pErrorSystem;
  return *pSystem;
}

inline FreeList<ErrorState>& GetErrorPool()
{
  return *(FreeList<ErrorState>*)GetErrorSystem().pErrorPool;
}

ErrorState* _AllocError(Result error, const SharedString &message, const char *function, const char *file, int line, ErrorState *pParent)
{
  ErrorState *pError = internal::GetErrorPool().Alloc();
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
  : pError(internal::_AllocError(error, message, function, file, line, pPrior))
{}

EPException::~EPException()
{
  FreeList<ErrorState>& pool = internal::GetErrorPool();
  while (pError)
  {
    ErrorState *pPrev = pError->pPrior;
    pError->Clear();
    pool.Free(pError);
    pError = pPrev;
  }
}


ErrorState* _PushError(Result error, const SharedString &message, const char *function, const char *file, int line)
{
  internal::ErrorSystem& errorSystem = internal::GetErrorSystem();
  errorSystem.pError = internal::_AllocError(error, message, function, file, line, errorSystem.pError);
  return errorSystem.pError;
}
ErrorState* GetError()
{
  return internal::GetErrorSystem().pError;
}

void ClearError()
{
  internal::ErrorSystem& errorSystem = internal::GetErrorSystem();
  FreeList<ErrorState>& pool = internal::GetErrorPool();
  while (errorSystem.pError)
  {
    ErrorState *pPrev = errorSystem.pError->pPrior;
    errorSystem.pError->Clear();
    pool.Free(errorSystem.pError);
    errorSystem.pError = pPrev;
  }
}

SharedString DumpError(ErrorState *pError)
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
