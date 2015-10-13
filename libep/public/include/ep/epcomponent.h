#pragma once
#if !defined(_EP_COMPONENT_H)
#define _EP_COMPONENT_H

#include "ep/epplugin.h"
#include "ep/epcomponentdesc.h"

#if defined(__cplusplus)
//extern "C" {
#endif

// component API
struct epComponent
{
  void *_vtbl;
  size_t refCount;

  const epComponentDesc *pType;
  struct epKernel *pKernel;
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
  epString(*GetUID)(epComponent *pComponent);
  epString(*GetName)(epComponent *pComponent);

  bool(*IsType)(epComponent *pComponent, epString type);

  epVariant(*GetProperty)(epComponent *pComponent, epString property);
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


/*
struct epNodePlugin
{
  epComponentOverrides component;

  void SetMatrix(const udDouble4x4 &mat) { matrix = mat; }
  void SetPosition(const udDouble3 &pos) { matrix.axis.t = udDouble4::create(pos, matrix.axis.t.w); }
  bool InputEvent(const epInputEvent &ev);
  bool Update(double timeStep);
  epResult Render(RenderSceneRef &spScene, const udDouble4x4 &mat);
};
*/

static inline int epComponent_Acquire(epComponent *pComponent)
{
  ++pComponent->refCount;
}

static inline int epComponent_Release(epComponent *pComponent)
{
  if (--pComponent->refCount == 0)
  {
    s_pPluginInstance->DestroyComponent(pComponent);
  }
}

static inline epString  epComponent_GetUID(epComponent *pComponent)                                                                 { return s_pPluginInstance->pComponentAPI->GetUID(pComponent); }
static inline epString  epComponent_GetName(epComponent *pComponent)                                                                { return s_pPluginInstance->pComponentAPI->GetName(pComponent); }
static inline bool      epComponent_IsType(epComponent *pComponent, epString type)                                                  { return s_pPluginInstance->pComponentAPI->IsType(pComponent, type); }
static inline epVariant epComponent_GetProperty(epComponent *pComponent, epString property)                                         { return s_pPluginInstance->pComponentAPI->GetProperty(pComponent, property); }
static inline void      epComponent_SetProperty(epComponent *pComponent, epString property, const epVariant *pValue)                { s_pPluginInstance->pComponentAPI->SetProperty(pComponent, property, pValue); }
static inline epVariant epComponent_CallMethod(epComponent *pComponent, epString method, const epVariant *pArgs, size_t numArgs)    { return s_pPluginInstance->pComponentAPI->CallMethod(pComponent, method, pArgs, numArgs); }
static inline void      epComponent_Subscribe(epComponent *pComponent, epString eventName, const epVarDelegate *pDelegate)          { s_pPluginInstance->pComponentAPI->Subscribe(pComponent, eventName, pDelegate); }
static inline epResult  epComponent_SendMessage(epComponent *pComponent, epString target, epString message, const epVariant *pData) { return s_pPluginInstance->pComponentAPI->SendMessage(pComponent, target, message, pData); }

#if defined(__cplusplus)
//}
#endif

#endif
