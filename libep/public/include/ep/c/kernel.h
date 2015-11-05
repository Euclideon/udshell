#if !defined(_EP_KERNEL_H)
#define _EP_KERNEL_H

#include "ep/c/component.h"

#if defined(__cplusplus)
extern "C" {
#endif

struct epKernel;

epResult     epKernel_SendMessage(epString target, epString sender, epString message, const epVariant* pData);
epResult     epKernel_RegisterComponentType(const epComponentDesc *pDesc);
epResult     epKernel_CreateComponent(epString typeId, const epKeyValuePair *pInitParams, size_t numInitParams, epComponent **ppNewInstance);
epComponent* epKernel_FindComponent(epString uid);
void         epKernel_Exec(epString code);
void         epKernel_LogError(const epString text, const epString componentUID);
void         epKernel_LogWarning(int level, const epString text, const epString componentUID);
void         epKernel_LogDebug(int level, const epString text, const epString componentUID);
void         epKernel_LogInfo(int level, const epString text, const epString componentUID);
void         epKernel_LogScript(const epString text, const epString componentUID);
void         epKernel_LogTrace(const epString text, const epString componentUID);

#if defined(__cplusplus)
}
#endif

#endif
