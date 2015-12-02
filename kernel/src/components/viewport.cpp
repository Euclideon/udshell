#include "viewport.h"

namespace kernel
{

// ---------------------------------------------------------------------------------------
Viewport::Viewport(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
  : UIComponent(pType, pKernel, uid, initParams)
{
  if (CreateInternal(initParams) != epR_Success)
    throw epR_Failure;
}

// ---------------------------------------------------------------------------------------
Viewport::~Viewport()
{
  DestroyInternal();
}

} // namespace kernel
