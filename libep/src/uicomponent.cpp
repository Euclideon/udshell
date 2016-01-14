#include "ep/cpp/component/uicomponent.h"

namespace ep {

UIComponent::UIComponent(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
  : Component(pType, pKernel, uid, initParams)
{
  pImpl = CreateImpl(initParams);
}

} // namespace ep
