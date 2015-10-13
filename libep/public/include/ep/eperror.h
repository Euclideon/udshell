#pragma once
#ifndef EPERROR_H
#define EPERROR_H

enum epResult
{
  epR_Success,
  epR_Failure_,

  epR_InvalidParameter_,
  epR_File_OpenFailure,
  epR_MemoryAllocationFailure,
};


#define EP_ERROR_BREAK_ON_ERROR 0  // Set to 1 to have the debugger break on error

#define EP_ERROR_IF(x, code)      do { if (x) { result = code; if (EP_ERROR_BREAK_ON_ERROR && code) { __debugbreak(); } goto epilogue; }                  } while(0)
#define EP_ERROR_NULL(ptr, code)  do { if (ptr == nullptr) { result = code; if (EP_ERROR_BREAK_ON_ERROR) { __debugbreak(); } goto epilogue; }             } while(0)
#define EP_ERROR_CHECK(funcCall)  do { result = funcCall; if (result != epR_Success) { if (EP_ERROR_BREAK_ON_ERROR) { __debugbreak(); } goto epilogue; }  } while(0)
#define EP_ERROR_HANDLE()         do { if (result != epR_Success) { if (EP_ERROR_BREAK_ON_ERROR) { __debugbreak(); } goto epilogue; }                     } while(0)

#if defined(EP_CPP11)

# include <functional>

  struct epErrorNode
  {
    epErrorNode(const std::function<void()> &error) : error(error) {}
    std::function<void()> error;
    epErrorNode *pNext = nullptr;
  };

  struct epError
  {
    epError(const epResult &result) : result(result) {}
    ~epError()
    {
      if (result != epR_Success)
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
    const epResult &result;
  };

# define EPERROR(__result) epError __error__(__result)
# define EPERROR_IF(x, __result, lambdaBody)  do { if (x) { result =__result;  return;} else { __error__.AddNode(new(alloca(sizeof(epErrorNode))) epErrorNode([&]()mutable lambdaBody));} } while(false);
# define EPERROR_NULL(ptr, __result, lambdaBody)  do {if (!ptr) { result =__result;  return;} else { __error__.AddNode(new(alloca(sizeof(epErrorNode))) epErrorNode([&]()mutable lambdaBody));} } while(false);
# define EPKNLERROR_CLEANUP(lambdaBody)  do {{ __error__.AddNode(new(alloca(sizeof(epErrorNode))) epErrorNode([&]()mutable lambdaBody));} } while(false);

#endif // defined(__cplusplus)

#endif // EPERROR_H
