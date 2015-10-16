#ifndef EPERROR_H
#define EPERROR_H

#if defined(__cplusplus)
extern "C" {
#endif

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

#if defined(__cplusplus)
}
#endif

#endif // EPERROR_H
