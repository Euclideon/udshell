#include "ep/cpp/component.h"
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
    s_pPluginInstance->DestroyComponent(pComponent);
    return 0;
  }
  return --pComponent->refCount;
}

epSharedString epComponent_GetUID(const epComponent *pComponent)
{
  return ((const Component*)pComponent)->GetUid();
}
epSharedString epComponent_GetName(const epComponent *pComponent)
{
  return ((const Component*)pComponent)->GetName();
}

bool epComponent_IsType(const epComponent *pComponent, epString type)
{
  return ((const Component*)pComponent)->IsType(type);
}

epVariant epComponent_GetProperty(const epComponent *pComponent, epString property)
{
  epVariant r;
  new(&r) Variant(((const Component*)pComponent)->GetProperty(property));
  return r;
}
void epComponent_SetProperty(epComponent *pComponent, epString property, const epVariant *pValue)
{
  ((Component*)pComponent)->SetProperty(property, *(Variant*)pValue);
}

epVariant epComponent_CallMethod(epComponent *pComponent, epString method, const epVariant *pArgs, size_t numArgs)
{
  epVariant r;
  new(&r) Variant(((Component*)pComponent)->CallMethod(method, Slice<const Variant>((const Variant*)pArgs, numArgs)));
  return r;
}

void epComponent_Subscribe(epComponent *pComponent, epString eventName, const epVarDelegate *pDelegate)
{
  ((Component*)pComponent)->Subscribe(eventName, (const Variant::VarDelegate&)pDelegate);
}

epResult epComponent_SendMessage(epComponent *pComponent, epString target, epString message, const epVariant *pData)
{
  return ((const Component*)pComponent)->SendMessage(target, message, *(Variant*)pData);
}

} // extern "C"

#include "../../kernel/src/componentdesc.h"

namespace ep {

ptrdiff_t epStringify(Slice<char> buffer, String format, Component *pComponent, const epVarArg *pArgs)
{
  ptrdiff_t len = pComponent->GetUid().length + 1;
  if (!buffer.ptr)
    return len;
  if (buffer.length < (size_t)len)
    return -len;

  return epStringifyTemplate(buffer, format, MutableString64(Concat, "@", pComponent->GetUid()), pArgs);
}


bool Component::IsType(String type) const
{
  const ComponentDesc *pDesc = pType;
  while (pDesc)
  {
    if (pDesc->info.id.eq(type))
      return true;
    pDesc = pDesc->pSuperDesc;
  }
  return false;
}

void Component::LogInternal(int category, int level, String text, String componentUID) const
{
  GetKernel().Log(category, level, text, componentUID);
}

epResult Component::ReceiveMessage(String message, String sender, const Variant &data)
{
  if (message.eqIC("set"))
  {
    Slice<Variant> arr = data.asArray();
    SetProperty(arr[0].asString(), arr[1]);
  }
  else if (message.eqIC("get"))
  {
    if (!sender.empty())
    {
      char mem[1024];
      Slice<char> buffer(mem, sizeof(mem));
//      GetProperty(data, &buffer);
//      SendMessage(sender, "val", buffer);
    }
  }
  return epR_Success;
}

epResult Component::SendMessage(String target, String message, const Variant &data) const
{
  return pKernel->SendMessage(target, uid, message, data);
}

} // namespace ep
