#pragma once
#ifndef EPERROR_HPP
#define EPERROR_HPP

#include "ep/c/error.h"
#include "ep/cpp/string.h"

#include <functional>

#define EPERROR(__result) ::ep::Error __error__(__result)
#define EPERROR_IF(x, __result, lambdaBody)  do { if (x) { result =__result;  return;} else { __error__.AddNode(new(alloca(sizeof(epErrorNode))) ::ep::ErrorNode([&]()mutable lambdaBody));} } while(false);
#define EPERROR_NULL(ptr, __result, lambdaBody)  do {if (!ptr) { result =__result;  return;} else { __error__.AddNode(new(alloca(sizeof(::ep::ErrorNode))) ::ep::ErrorNode([&]()mutable lambdaBody));} } while(false);
#define EPKNLERROR_CLEANUP(lambdaBody)  do {{ __error__.AddNode(new(alloca(sizeof(::ep::ErrorNode))) ::ep::ErrorNode([&]()mutable lambdaBody));} } while(false);

namespace ep {

#define PushError(error, message) epPushError(error, message, __FILE__, __LINE__)
epErrorState* GetError();
void ClearError();

SharedString DumpError();


struct ErrorNode
{
  ErrorNode(const std::function<void()> &error) : error(error) {}
  std::function<void()> error;
  ErrorNode *pNext = nullptr;
};

struct Error
{
  Error(const epResult &result) : result(result) {}
  ~Error()
  {
    if (result != epR_Success)
    {
      ErrorNode *pNode = pRoot;
      while (pNode)
      {
        ErrorNode *pCurrent = pNode;
        pNode = pNode->pNext;

        pCurrent->error();
        pCurrent->~ErrorNode();
      }
    }
  }

  void operator=(const Error &) = delete;
  void AddNode(ErrorNode *pNode)
  {
    pNode->pNext = pRoot;
    pRoot = pNode;
  }
  ErrorNode *pRoot = nullptr;
  const epResult &result;
};



// impl...

namespace internal {
  extern thread_local epErrorState s_errorStack[256];
  extern thread_local size_t s_errorDepth;
}

inline epErrorState* GetError()
{
  return internal::s_errorDepth ? &internal::s_errorStack[internal::s_errorDepth-1] : nullptr;
}
inline void ClearError()
{
  epClearError();
}

inline SharedString DumpError()
{
  return epDumpError();
}

} // namespace ep

#endif // EPERROR_HPP
