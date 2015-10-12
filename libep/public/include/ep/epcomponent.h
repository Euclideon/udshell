#pragma once
#if !defined(_EP_COMPONENT_H)
#define _EP_COMPONENT_H

#include "ep/epplugin.h"
#include "ep/epcomponentdesc.h"

//extern "C" {

// component API
struct epComponent
{
  void *_vtbl;
  size_t refCount;

  const epComponentDesc *pType;
  struct epKernel *pKernel;
};

inline int epAcquire(epComponent *pComponent)
{
  ++pComponent->refCount;
}

inline int epRelease(epComponent *pComponent)
{
  if (--pComponent->refCount == 0)
  {
    s_pPluginInstance->DestroyComponent(pComponent);
  }
}


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


// component virtuals, used to derive from component
struct epComponentOverrides
{
  size_t structSize;

  void* (*pCreateInstance)(epComponent *pBaseInstance, const epKeyValuePair *pInitParams, size_t numInitParams);
  void (*pDestroy)(epComponent *pBaseInstance, void *pDerivedInstance);
  epResult (*pInitComplete)(epComponent *pBaseInstance, void *pDerivedInstance);
  epResult (*pReceiveMessage)(epComponent *pBaseInstance, void *pDerivedInstance, epString message, epString sender, const epVariant *pData);
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

//}

#endif
