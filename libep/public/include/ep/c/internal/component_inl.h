#if !defined(_EP_COMPONENT_INL_H)
#define _EP_COMPONENT_INL_H

#include "ep/c/component.h"

struct epComponentAPI
{
  epSharedString(*GetUID)(const epComponent *pComponent);
  epSharedString(*GetName)(const epComponent *pComponent);

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

#endif // _EP_COMPONENT_INL_H
