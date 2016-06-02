#include "ep/cpp/plugin.h"
#include "ep/cpp/kernel.h"

extern "C" {

void epKernel_SendMessage(epString target, epString sender, epString message, const epVariant* pData)
{
  using namespace ep;
  try {
    Kernel::GetInstance()->SendMessage(target, sender, message, *(Variant*)pData);
  } catch (std::exception &e) {
    Kernel::GetInstance()->LogError("Message Handler {0} failed: {1}", (String&)target, e.what());
  } catch (...) {
    Kernel::GetInstance()->LogError("Message Handler {0} failed", (String&)target);
  }
}

void epKernel_RegisterComponentType(const epComponentDesc *pDesc)
{
  EPASSERT(false, "TODO");
/*
  class CComponent : public ep::Component
  {
  };

  if (!Kernel::GetInstance()->RegisterComponentType<CComponent>())
    return Result::Failure;
  return Result::Success;
*/
}

void epKernel_CreateComponent(epString typeId, const epKeyValuePair *pInitParams, size_t numInitParams, epComponent **ppNewInstance)
{
  try
  {
    using namespace ep;
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
  epConstruct(&r) ep::ComponentRef(ep::Kernel::GetInstance()->FindComponent(uid));
  return r;
}

void epKernel_Exec(epString code)
{
  ep::Kernel::GetInstance()->Exec(code);
}

void epKernel_LogError(const epString text, const epString componentUID)
{
  ep::Kernel::GetInstance()->LogError(ep::String(text), ep::String(componentUID));
}

void epKernel_LogWarning(int level, const epString text, const epString componentUID)
{
  ep::Kernel::GetInstance()->LogWarning(level, ep::String(text), ep::String(componentUID));
}

void epKernel_LogDebug(int level, const epString text, const epString componentUID)
{
  ep::Kernel::GetInstance()->LogDebug(level, ep::String(text), ep::String(componentUID));
}

void epKernel_LogInfo(int level, const epString text, const epString componentUID)
{
  ep::Kernel::GetInstance()->LogInfo(level, ep::String(text), ep::String(componentUID));
}

void epKernel_LogScript(const epString text, const epString componentUID)
{
  ep::Kernel::GetInstance()->LogScript(ep::String(text), ep::String(componentUID));
}

void epKernel_LogTrace(const epString text, const epString componentUID)
{
  ep::Kernel::GetInstance()->LogTrace(ep::String(text), ep::String(componentUID));
}

}
