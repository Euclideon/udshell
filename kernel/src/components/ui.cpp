#include "ui.h"

namespace ep
{

ComponentDesc UIComponent::descriptor =
{
  &Component::descriptor, // pSuperDesc

  UDSHELL_APIVERSION,     // udVersion
  UDSHELL_PLUGINVERSION,  // pluginVersion

  "ui",                // id
  "UI",                // displayName
  "Is a UI component", // description
};


UIComponent::UIComponent(const ComponentDesc *pType, Kernel *pKernel, epSharedString uid, epInitParams initParams)
  : Component(pType, pKernel, uid, initParams)
{
  if (CreateInternal(initParams) != udR_Success)
    throw udR_Failure_;
}

UIComponent::~UIComponent()
{
  DestroyInternal();
}

} // namespace ep
