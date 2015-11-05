#if !defined(_EPDELEGATE_H)
#define _EPDELEGATE_H

#include "ep/c/variant.h"

#if defined(__cplusplus)
extern "C" {
#endif

struct epVarDelegate;

typedef epVariant(epVarMethod)(void *pThis, const epVariant *pArgs, size_t numArgs);

size_t epVarDelegate_Acquire(epVarDelegate *pDelegate);
size_t epVarDelegate_Release(epVarDelegate *pDelegate);
epVariant epVarDelegate_Call(epVarDelegate *pDelegate, const epVariant *pArgs, size_t numArgs);
epVarDelegate* epVarDelegate_Make(void *pThis, epVarMethod *pFunc);

#if defined(__cplusplus)
}
#endif

#endif // _EPDELEGATE_H
