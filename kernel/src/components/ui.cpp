#include "ui.h"

namespace ep
{

ComponentDesc UIComponent::descriptor =
{
  &Component::descriptor, // pSuperDesc

  EPSHELL_APIVERSION,     // epVersion
  EPSHELL_PLUGINVERSION,  // pluginVersion

  "ui",                // id
  "UI",                // displayName
  "Is a UI component", // description
};


UIComponent::UIComponent(const ComponentDesc *pType, Kernel *pKernel, epSharedString uid, epInitParams initParams)
  : Component(pType, pKernel, uid, initParams)
{
  if (CreateInternal(initParams) != epR_Success)
    throw epR_Failure_;
}

UIComponent::~UIComponent()
{
  DestroyInternal();
}

} // namespace ep
