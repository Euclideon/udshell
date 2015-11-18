#pragma once
#ifndef EPCOMPONENTPLUGIN_H
#define EPCOMPONENTPLUGIN_H

#include "components/component.h"
#include "ep/c/component.h"
#include "ep/c/internal/component_inl.h"

namespace ep {

class ComponentPlugin : public Component
{
public:
  EP_COMPONENT(ComponentPlugin);


private:
  ComponentPlugin(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, InitParams initParams)
    : Component(pType, pKernel, uid, initParams)
  {
    pCallbacks = pType->pExternalDesc->pOverrides;
    if (pCallbacks->pCreateInstance)
    {
      pUserData = pCallbacks->pCreateInstance((epComponent*)this, (const epKeyValuePair*)initParams.params.ptr, initParams.params.length);
      if (!pUserData)
        throw epR_Failure;
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

  epResult ReceiveMessage(String message, String sender, const Variant &data) override
  {
    if (pCallbacks->pReceiveMessage)
      return pCallbacks->pReceiveMessage((epComponent*)this, pUserData, message, sender, (const epVariant*)&data);
    return epR_Success;
  }

  const epComponentOverrides *pCallbacks;
};

} // namespace ep

#endif // EPCOMPONENTPLUGIN_H
