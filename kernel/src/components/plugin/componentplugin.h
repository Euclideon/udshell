#pragma once
#ifndef EPCOMPONENTPLUGIN_H
#define EPCOMPONENTPLUGIN_H

#include "components/component.h"
#include "ep/epcomponent.h"

namespace ep
{

class ComponentPlugin : public Component
{
public:
  EP_COMPONENT(ComponentPlugin);


private:
  ComponentPlugin(const ComponentDesc *pType, Kernel *_pKernel, epSharedString _uid, epInitParams initParams)
    : Component(pType, pKernel, uid, initParams)
  {
    pUserData = pCallbacks->pCreateInstance((epComponent*)this, initParams.params.ptr, initParams.params.length);
    if (!pUserData)
      throw udR_Failure_;
  }
  virtual ~ComponentPlugin()
  {
    pCallbacks->pDestroy((epComponent*)this, pUserData);
  }

  udResult InitComplete() override
  {
    return pCallbacks->pInitComplete((epComponent*)this, pUserData);
  }

  udResult ReceiveMessage(epString message, epString sender, const epVariant &data) override
  {
    return pCallbacks->pReceiveMessage((epComponent*)this, pUserData, message, sender, &data);
  }

  epComponentOverrides *pCallbacks;
};

} // namespace ep

#endif // EPCOMPONENTPLUGIN_H
