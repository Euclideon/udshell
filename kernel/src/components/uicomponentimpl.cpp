#include "components/uicomponentimpl.h"

namespace ep {

UIComponentImpl::UIComponentImpl(Component *pInstance, Variant::VarMap initParams)
  : Super(pInstance)
{
  if (CreateInternal(initParams) != epR_Success)
    EPTHROW_ERROR(epR_Failure, "Failed to create UIComponent");
}

} // namespace ep
