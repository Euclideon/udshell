#include "viewport.h"

namespace ep
{

// ---------------------------------------------------------------------------------------
Viewport::Viewport(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
  : UIComponent(pType, pKernel, uid, initParams)
{
  if (CreateInternal(initParams) != epR_Success)
    EPTHROW_ERROR(epR_Failure, "Failed to create Viewport");
}

// ---------------------------------------------------------------------------------------
Viewport::~Viewport()
{
  DestroyInternal();
}

} // namespace ep
