#include "viewport.h"

namespace ud
{

ComponentDesc Viewport::descriptor =
{
  &Component::descriptor, // pSuperDesc

  UDSHELL_APIVERSION,     // udVersion
  UDSHELL_PLUGINVERSION,  // pluginVersion

  "viewport",             // id
  "Viewport",             // displayName
  "Is a viewport",        // description
};


// ---------------------------------------------------------------------------------------
Viewport::Viewport(const ComponentDesc *pType, Kernel *pKernel, udSharedString uid, udInitParams initParams)
  : UIComponent(pType, pKernel, uid, initParams)
{
  if (CreateInternal(initParams) != udR_Success)
    throw udR_Failure_;
}

// ---------------------------------------------------------------------------------------
Viewport::~Viewport()
{
  DestroyInternal();
}

} // namespace ud
