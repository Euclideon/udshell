#include "libep_internal.h"
#include "ep/cpp/component/component.h"
#include "ep/cpp/componentdesc.h"
#include "ep/cpp/kernel.h"

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
