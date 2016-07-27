#include "libep_internal.h"
#include "ep/cpp/component/component.h"
#include "ep/cpp/componentdesc.h"
#include "ep/cpp/kernel.h"

namespace ep {

ptrdiff_t epStringify(Slice<char> buffer, String format, const Component *pComponent, const VarArg *pArgs)
{
  ptrdiff_t len = pComponent->getUid().length + 1;
  if (!buffer.ptr)
    return len;
  if (buffer.length < (size_t)len)
    return -len;

  return epStringifyTemplate(buffer, format, MutableString64(Concat, "@", pComponent->getUid()), pArgs);
}


Component::Component(const ComponentDesc *_pType, Kernel *_pKernel, SharedString _uid, Variant::VarMap initParams)
  : uid(_uid), pType(_pType), pKernel(_pKernel)
{
  pImpl = createImpl(initParams);
}

bool Component::isType(String type) const
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

void* Component::createImplInternal(String componentType, Variant::VarMap initParams)
{
  if(pKernel)
    return pKernel->createImpl(componentType, this, initParams);
  return nullptr;
}

void Component::sendMessage(String target, String message, const Variant &data) const
{
  return pKernel->SendMessage(target, uid, message, data);
}

} // namespace ep
