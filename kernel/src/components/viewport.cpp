#include "viewport.h"

namespace ep
{

ComponentDesc Viewport::descriptor =
{
  &UIComponent::descriptor, // pSuperDesc

  EPSHELL_APIVERSION,     // epVersion
  EPSHELL_PLUGINVERSION,  // pluginVersion

  "viewport",             // id
  "Viewport",             // displayName
  "Is a viewport",        // description
};


// ---------------------------------------------------------------------------------------
Viewport::Viewport(const ComponentDesc *pType, Kernel *pKernel, epSharedString uid, epInitParams initParams)
  : UIComponent(pType, pKernel, uid, initParams)
{
  if (CreateInternal(initParams) != epR_Success)
    throw epR_Failure_;
}

// ---------------------------------------------------------------------------------------
Viewport::~Viewport()
{
  DestroyInternal();
}

} // namespace ep
