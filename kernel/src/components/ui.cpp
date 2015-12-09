#include "ui.h"

namespace ep {

UIComponent::UIComponent(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
  : Component(pType, pKernel, uid, initParams)
{
  if (CreateInternal(initParams) != epR_Success)
    throw epR_Failure;
}

UIComponent::~UIComponent()
{
  DestroyInternal();
}

} // namespace ep
