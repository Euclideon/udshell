#include "ep/cpp/component.h"

extern "C" {

int epComponent_Acquire(epComponent *pComponent)
{
  return (int)(++pComponent->refCount);
}

int epComponent_Release(epComponent *pComponent)
{
  if (--pComponent->refCount == 0)
  {
    s_pPluginInstance->DestroyComponent(pComponent);
    return 0;
  }
  return (int)pComponent->refCount;
}

epString epComponent_GetUID(const epComponent *pComponent)
{
  return s_pPluginInstance->pComponentAPI->GetUID(pComponent);
}
epString epComponent_GetName(const epComponent *pComponent)
{
  return s_pPluginInstance->pComponentAPI->GetName(pComponent);
}

bool epComponent_IsType(const epComponent *pComponent, epString type)
{
  return s_pPluginInstance->pComponentAPI->IsType(pComponent, type);
}

epVariant epComponent_GetProperty(const epComponent *pComponent, epString property)
{
  return s_pPluginInstance->pComponentAPI->GetProperty(pComponent, property);
}
void epComponent_SetProperty(epComponent *pComponent, epString property, const epVariant *pValue)
{
  s_pPluginInstance->pComponentAPI->SetProperty(pComponent, property, pValue);
}

epVariant epComponent_CallMethod(epComponent *pComponent, epString method, const epVariant *pArgs, size_t numArgs)
{
  return s_pPluginInstance->pComponentAPI->CallMethod(pComponent, method, pArgs, numArgs);
}

void epComponent_Subscribe(epComponent *pComponent, epString eventName, const epVarDelegate *pDelegate)
{
  s_pPluginInstance->pComponentAPI->Subscribe(pComponent, eventName, pDelegate);
}

epResult epComponent_SendMessage(epComponent *pComponent, epString target, epString message, const epVariant *pData)
{
  return s_pPluginInstance->pComponentAPI->SendMessage(pComponent, target, message, pData);
}

} // extern "C"

namespace ep {

ptrdiff_t epStringify(Slice<char> buffer, String format, Component *pComponent, const epVarArg *pArgs)
{
  ptrdiff_t len = pComponent->GetUID().length + 1;
  if (!buffer.ptr)
    return len;
  if (buffer.length < (size_t)len)
    return -len;

  // HACK: this could be a lot nicer!
  MutableString64 uid; uid.concat("@", pComponent->GetUID());
  return epStringifyTemplate(buffer, format, uid, pArgs);
}

} // namespace ep
