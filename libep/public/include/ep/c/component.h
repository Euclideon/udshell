#if !defined(_EP_COMPONENT_H)
#define _EP_COMPONENT_H

#include "ep/c/plugin.h"
#include "ep/c/componentdesc.h"

#if defined(__cplusplus)
extern "C" {
#endif

// component API
struct epComponent
{
  void *_vtbl;
  size_t refCount;

  const epComponentDesc *pType;
  struct epKernel *pKernel;

  epString uid;
};


// component virtuals, used to derive from component
struct epComponentOverrides
{
  size_t structSize;

  void*(*pCreateInstance)(epComponent *pBaseInstance, const epKeyValuePair *pInitParams, size_t numInitParams);
  void(*pDestroy)(epComponent *pBaseInstance, void *pDerivedInstance);
  epResult(*pInitComplete)(epComponent *pBaseInstance, void *pDerivedInstance);
  epResult(*pReceiveMessage)(epComponent *pBaseInstance, void *pDerivedInstance, epString message, epString sender, const epVariant *pData);
};


// fast-access API for component
struct epComponentAPI
{
  epString(*GetUID)(const epComponent *pComponent);
  epString(*GetName)(const epComponent *pComponent);

  bool(*IsType)(const epComponent *pComponent, epString type);

  epVariant(*GetProperty)(const epComponent *pComponent, epString property);
  void(*SetProperty)(epComponent *pComponent, epString property, const epVariant *pValue);

  epVariant(*CallMethod)(epComponent *pComponent, epString method, const epVariant *pArgs, size_t numArgs);

  void(*Subscribe)(epComponent *pComponent, epString eventName, const epVarDelegate *pDelegate);

  epResult(*SendMessage)(epComponent *pComponent, epString target, epString message, const epVariant *pData);
/*
  const PropertyInfo *(*GetPropertyInfo)(epComponent *pComponent, epString name);
  const FunctionInfo *(*GetMethodInfo)(epComponent *pComponent, epString name);
  const EventInfo *(*GetEventInfo)(epComponent *pComponent, epString name);
  const FunctionInfo *(*GetStaticFuncInfo)(epComponent *pComponent, epString name);
*/
};


inline int epComponent_Acquire(epComponent *pComponent)
{
  ++pComponent->refCount;
}

inline int epComponent_Release(epComponent *pComponent)
{
  if (--pComponent->refCount == 0)
  {
    s_pPluginInstance->DestroyComponent(pComponent);
  }
}

inline epString  epComponent_GetUID(const epComponent *pComponent)                                                                 { return s_pPluginInstance->pComponentAPI->GetUID(pComponent); }
inline epString  epComponent_GetName(const epComponent *pComponent)                                                                { return s_pPluginInstance->pComponentAPI->GetName(pComponent); }
inline bool      epComponent_IsType(const epComponent *pComponent, epString type)                                                  { return s_pPluginInstance->pComponentAPI->IsType(pComponent, type); }
inline epVariant epComponent_GetProperty(const epComponent *pComponent, epString property)                                         { return s_pPluginInstance->pComponentAPI->GetProperty(pComponent, property); }
inline void      epComponent_SetProperty(epComponent *pComponent, epString property, const epVariant *pValue)                { s_pPluginInstance->pComponentAPI->SetProperty(pComponent, property, pValue); }
inline epVariant epComponent_CallMethod(epComponent *pComponent, epString method, const epVariant *pArgs, size_t numArgs)    { return s_pPluginInstance->pComponentAPI->CallMethod(pComponent, method, pArgs, numArgs); }
inline void      epComponent_Subscribe(epComponent *pComponent, epString eventName, const epVarDelegate *pDelegate)          { s_pPluginInstance->pComponentAPI->Subscribe(pComponent, eventName, pDelegate); }
inline epResult  epComponent_SendMessage(epComponent *pComponent, epString target, epString message, const epVariant *pData) { return s_pPluginInstance->pComponentAPI->SendMessage(pComponent, target, message, pData); }

#if defined(__cplusplus)
}
#endif

#endif
