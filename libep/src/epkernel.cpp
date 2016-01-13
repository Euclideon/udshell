#include "ep/cpp/plugin.h"
#include "ep/cpp/kernel.h"

extern "C" {

epResult epKernel_SendMessage(epString target, epString sender, epString message, const epVariant* pData)
{
  return Kernel::GetInstance()->SendMessage(target, sender, message, *(Variant*)pData);
}

epResult epKernel_RegisterComponentType(const epComponentDesc *pDesc)
{
  EPASSERT(false, "TODO");
/*
  class CComponent : public ep::Component
  {
  };

  if (!Kernel::GetInstance()->RegisterComponentType<CComponent>())
    return epR_Failure;
  return epR_Success;
*/
  return epR_Failure;
}

epResult epKernel_CreateComponent(epString typeId, const epKeyValuePair *pInitParams, size_t numInitParams, epComponent **ppNewInstance)
{
  return Kernel::GetInstance()->CreateComponent(typeId, Slice<const KeyValuePair>((const KeyValuePair*)pInitParams, numInitParams), (ComponentRef*)ppNewInstance);
}

epComponent* epKernel_FindComponent(epString uid)
{
  epComponent* r;
  new(&r) ComponentRef(Kernel::GetInstance()->FindComponent(uid));
  return r;
}

void epKernel_Exec(epString code)
{
  Kernel::GetInstance()->Exec(code);
}

void epKernel_LogError(const epString text, const epString componentUID)
{
  Kernel::GetInstance()->LogError(String(text), String(componentUID));
}

void epKernel_LogWarning(int level, const epString text, const epString componentUID)
{
  Kernel::GetInstance()->LogWarning(level, String(text), String(componentUID));
}

void epKernel_LogDebug(int level, const epString text, const epString componentUID)
{
  Kernel::GetInstance()->LogDebug(level, String(text), String(componentUID));
}

void epKernel_LogInfo(int level, const epString text, const epString componentUID)
{
  Kernel::GetInstance()->LogInfo(level, String(text), String(componentUID));
}

void epKernel_LogScript(const epString text, const epString componentUID)
{
  Kernel::GetInstance()->LogScript(String(text), String(componentUID));
}

void epKernel_LogTrace(const epString text, const epString componentUID)
{
  Kernel::GetInstance()->LogTrace(String(text), String(componentUID));
}

}
