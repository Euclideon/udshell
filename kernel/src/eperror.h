#pragma once
#ifndef UDERROR_H
#define UDERROR_H
#include <functional>
struct epErrorNode
{
  epErrorNode(const std::function<void()> &error) : error(error) {}
  std::function<void()> error;
  epErrorNode *pNext = nullptr;
};

struct epError
{
  epError(const udResult &result) : result(result) {}
  ~epError()
  {
    if (result != udR_Success)
    {
      epErrorNode *pNode = pRoot;
      while (pNode)
      {
        epErrorNode *pCurrent = pNode;
        pNode = pNode->pNext;

        pCurrent->error();
        pCurrent->~epErrorNode();
      }
    }
  }

  void operator=(const epError &) = delete;
  void AddNode(epErrorNode *pNode)
  {
    pNode->pNext = pRoot;
    pRoot = pNode;
  }
  epErrorNode *pRoot = nullptr;
  const udResult &result;
};

#define EPERROR(__result) epError __error__(__result)
#define EPERROR_IF(x, __result, lambdaBody)  do { if (x) { result =__result;  return;} else { __error__.AddNode(new(alloca(sizeof(epErrorNode))) epErrorNode([&]()mutable lambdaBody));} } while(false);
#define EPERROR_NULL(ptr, __result, lambdaBody)  do {if (!ptr) { result =__result;  return;} else { __error__.AddNode(new(alloca(sizeof(epErrorNode))) epErrorNode([&]()mutable lambdaBody));} } while(false);
#define EPKNLERROR_CLEANUP(lambdaBody)  do {{ __error__.AddNode(new(alloca(sizeof(epErrorNode))) epErrorNode([&]()mutable lambdaBody));} } while(false);


#endif // UDERROR_H
