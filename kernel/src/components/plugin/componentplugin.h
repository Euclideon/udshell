#pragma once
#ifndef EPCOMPONENTPLUGIN_H
#define EPCOMPONENTPLUGIN_H

#include "components/component.h"
#include "ep/epcomponent.h"

namespace ep {

class ComponentPlugin : public Component
{
public:
  EP_COMPONENT(ComponentPlugin);


private:
  ComponentPlugin(const ComponentDesc *pType, Kernel *pKernel, epSharedString uid, epInitParams initParams)
    : Component(pType, pKernel, uid, initParams)
  {
    pCallbacks = pType->pExternalDesc->pOverrides;
    if (pCallbacks->pCreateInstance)
    {
      pUserData = pCallbacks->pCreateInstance((epComponent*)this, initParams.params.ptr, initParams.params.length);
      if (!pUserData)
        throw epR_Failure_;
    }
  }
  virtual ~ComponentPlugin()
  {
    if (pCallbacks->pDestroy)
      pCallbacks->pDestroy((epComponent*)this, pUserData);
  }

  epResult InitComplete() override
  {
    if (pCallbacks->pInitComplete)
      return pCallbacks->pInitComplete((epComponent*)this, pUserData);
    return epR_Success;
  }

  epResult ReceiveMessage(epString message, epString sender, const epVariant &data) override
  {
    if (pCallbacks->pReceiveMessage)
      return pCallbacks->pReceiveMessage((epComponent*)this, pUserData, message, sender, &data);
    return epR_Success;
  }

  epComponentOverrides *pCallbacks;
};

} // namespace ep

#endif // EPCOMPONENTPLUGIN_H
