#include "ep/cpp/interface/icomponent.h"

namespace ep {

epComponentOverrides IComponent::GetOverrides()
{
  epComponentOverrides overrides = {
    sizeof(epComponentOverrides),
    nullptr,
    [](epComponent *pBaseInstance, void *pDerivedInstance)
    {
      delete (IComponent*)pDerivedInstance;
    },
    [](epComponent *pBaseInstance, void *pDerivedInstance) -> epResult
    {
      IComponent *pI = (IComponent*)pDerivedInstance;
      return pI->InitComplete();
    },
    [](epComponent *pBaseInstance, void *pDerivedInstance, epString message, epString sender, const epVariant *pData) -> epResult
    {
      IComponent *pI = (IComponent*)pDerivedInstance;
      return pI->ReceiveMessage(message, sender, *(const Variant*)pData);
    }
  };
  return overrides;
}

} // namespace ep
