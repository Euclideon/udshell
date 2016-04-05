#include "ep/cpp/component/component.h"
#include "ep/cpp/kernel.h"

extern "C" {

size_t epComponent_Acquire(epComponent *pComponent)
{
  return ++pComponent->refCount;
}

size_t epComponent_Release(epComponent *pComponent)
{
  if (pComponent->refCount == 1)
  {
    ep::s_pInstance->DestroyComponent((ep::Component*)pComponent); // TODO: this can go, replace with normal DecRef, pFree will take care of it
    return 0;
  }
  return --pComponent->refCount;
}

epSharedString epComponent_GetUID(const epComponent *pComponent)
{
  return ((const ep::Component*)pComponent)->GetUid();
}
epSharedString epComponent_GetName(const epComponent *pComponent)
{
  return ((const ep::Component*)pComponent)->GetName();
}

bool epComponent_IsType(const epComponent *pComponent, epString type)
{
  return ((const ep::Component*)pComponent)->IsType(type);
}

epVariant epComponent_GetProperty(const epComponent *pComponent, epString property)
{
  epVariant r;
  new(&r) ep::Variant(((const ep::Component*)pComponent)->Get(property));
  return r;
}
void epComponent_SetProperty(epComponent *pComponent, epString property, const epVariant *pValue)
{
  ((ep::Component*)pComponent)->Set(property, *(ep::Variant*)pValue);
}

epVariant epComponent_CallMethod(epComponent *pComponent, epString method, const epVariant *pArgs, size_t numArgs)
{
  epVariant r;
  new(&r) ep::Variant(((ep::Component*)pComponent)->Call(method, ep::Slice<const ep::Variant>((const ep::Variant*)pArgs, numArgs)));
  return r;
}

void epComponent_Subscribe(epComponent *pComponent, epString eventName, const epVarDelegate *pDelegate)
{
  ((ep::Component*)pComponent)->Subscribe(eventName, (const ep::VarDelegate&)pDelegate);
}

void epComponent_SendMessage(epComponent *pComponent, epString target, epString message, const epVariant *pData)
{
  try { ((const ep::Component*)pComponent)->SendMessage(target, message, *(ep::Variant*)pData); } catch (...) {}
}

} // extern "C"

#include "ep/cpp/componentdesc.h"

namespace ep {

ptrdiff_t epStringify(Slice<char> buffer, String format, const Component *pComponent, const VarArg *pArgs)
{
  ptrdiff_t len = pComponent->GetUid().length + 1;
  if (!buffer.ptr)
    return len;
  if (buffer.length < (size_t)len)
    return -len;

  return epStringifyTemplate(buffer, format, MutableString64(Concat, "@", pComponent->GetUid()), pArgs);
}


Component::Component(const ComponentDesc *_pType, Kernel *_pKernel, SharedString _uid, Variant::VarMap initParams)
  : uid(_uid), pType(_pType), pKernel(_pKernel)
{
  pImpl = CreateImpl(initParams);
}

bool Component::IsType(String type) const
{
  const ComponentDesc *pDesc = pType;
  while (pDesc)
  {
    if (pDesc->info.identifier.eq(type))
      return true;
    pDesc = pDesc->pSuperDesc;
  }
  return false;
}

void* Component::CreateImplInternal(String componentType, Variant::VarMap initParams)
{
  if(pKernel)
    return pKernel->CreateImpl(componentType, this, initParams);
  return nullptr;
}

void Component::SendMessage(String target, String message, const Variant &data) const
{
  return pKernel->SendMessage(target, uid, message, data);
}

} // namespace ep
