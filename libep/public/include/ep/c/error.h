#ifndef EPERROR_H
#define EPERROR_H

#if defined(__cplusplus)
extern "C" {
#endif

#include "ep/c/string.h"

enum epResult
{
  epR_Success,
  epR_Failure,

  epR_CppException,
  epR_InvalidCall,
  epR_InvalidArgument,
  epR_InvalidType,
  epR_OutOfBounds,
  epR_BadCast,
  epR_AllocFailure,
  epR_ResourceInUse,
  epR_AlreadyExists,

  epR_File_OpenFailure,

  epR_ScriptException,
};

struct epErrorState
{
  epResult error;
  epString message;

  epString file;
  int line;

  epErrorState *pPrior;
};

#define EPERROR(error, message) epPushError(error, message, __PRETTY_FUNCTION__, __FILE__, __LINE__)

epErrorState* epPushError(epResult error, epString message, const char *function, const char *file, int line);
epErrorState* epGetError();
void epClearError();

epSharedString epDumpError();


// TODO: deprecate and remove these...
#define EP_ERROR_BREAK_ON_ERROR 0  // Set to 1 to have the debugger break on error
#define EP_ERROR_IF(x, code)      do { if (x) { result = code; if (EP_ERROR_BREAK_ON_ERROR && code) { __debugbreak(); } goto epilogue; }                  } while(0)
#define EP_ERROR_NULL(ptr, code)  do { if (ptr == nullptr) { result = code; if (EP_ERROR_BREAK_ON_ERROR) { __debugbreak(); } goto epilogue; }             } while(0)
#define EP_ERROR_CHECK(funcCall)  do { result = funcCall; if (result != epR_Success) { if (EP_ERROR_BREAK_ON_ERROR) { __debugbreak(); } goto epilogue; }  } while(0)
#define EP_ERROR_HANDLE()         do { if (result != epR_Success) { if (EP_ERROR_BREAK_ON_ERROR) { __debugbreak(); } goto epilogue; }                     } while(0)

#if defined(__cplusplus)
}
#endif

#endif // EPERROR_H
