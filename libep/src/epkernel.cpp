#include "ep/cpp/plugin.h"
#include "ep/cpp/kernel.h"

extern "C" {

void epKernel_SendMessage(epString target, epString sender, epString message, const epVariant* pData)
{
  try {
    Kernel::GetInstance()->SendMessage(target, sender, message, *(Variant*)pData);
  } catch (std::exception &e) {
    Kernel::GetInstance()->LogError("Message Handler {0} failed: {1}", (ep::String&)target, e.what());
  } catch (...) {
    Kernel::GetInstance()->LogError("Message Handler {0} failed", (ep::String&)target);
  }
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

void epKernel_CreateComponent(epString typeId, const epKeyValuePair *pInitParams, size_t numInitParams, epComponent **ppNewInstance)
{
  try
  {
    ComponentRef spCom = Kernel::GetInstance()->CreateComponent(typeId, Slice<const KeyValuePair>((const KeyValuePair*)pInitParams, numInitParams));
    *ppNewInstance = (epComponent*)spCom.ptr();
  }
  catch (...)
  {
    *ppNewInstance = nullptr;
  }
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
