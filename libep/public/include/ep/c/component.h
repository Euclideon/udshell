#if !defined(_EP_COMPONENT_H)
#define _EP_COMPONENT_H

#include "ep/c/plugin.h"
#include "ep/c/componentdesc.h"

#if defined(__cplusplus)
extern "C" {
#endif

typedef void (epFreeFunc)(void*);

// component API
struct epComponent
{
  const void *_vtbl;
  size_t refCount;
  epFreeFunc *pFreeFunc;

  const epComponentDesc *pType;
  struct epKernel *pKernel;

  const epSharedString uid;
  epSharedString name;

  void *pUserData;
};


// component virtuals, used to derive from component
struct epComponentOverrides
{
  size_t structSize;

  // TODO: these are probably out of date
  void*(*pCreateInstance)(epComponent *pBaseInstance, const epVarMap *pInitParams);
  void(*pDestroy)(epComponent *pBaseInstance, void *pDerivedInstance);
  void(*pInitComplete)(epComponent *pBaseInstance, void *pDerivedInstance);
  void(*pReceiveMessage)(epComponent *pBaseInstance, void *pDerivedInstance, epString message, epString sender, const epVariant *pData);
};

size_t epComponent_Acquire(epComponent *pComponent);
size_t epComponent_Release(epComponent *pComponent);

epSharedString epComponent_GetUID(const epComponent *pComponent);
epSharedString epComponent_GetName(const epComponent *pComponent);
bool      epComponent_IsType(const epComponent *pComponent, epString type);
epVariant epComponent_GetProperty(const epComponent *pComponent, epString property);
void      epComponent_SetProperty(epComponent *pComponent, epString property, const epVariant *pValue);
epVariant epComponent_CallMethod(epComponent *pComponent, epString method, const epVariant *pArgs, size_t numArgs);
void      epComponent_Subscribe(epComponent *pComponent, epString eventName, const epVarDelegate *pDelegate);
void      epComponent_SendMessage(epComponent *pComponent, epString target, epString message, const epVariant *pData);

#if defined(__cplusplus)
}
#endif

#endif
