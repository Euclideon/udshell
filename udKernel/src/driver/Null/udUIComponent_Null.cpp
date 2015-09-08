#include "hal/driver.h"

#if UDUI_DRIVER == UDDRIVER_NULL

#include "components/ui.h"

namespace ud
{

Component *UIComponent::Create(const ComponentDesc *pType, Kernel *pKernel, udSharedString uid, udInitParams initParams)
{
  return nullptr;
}

Component *Window::Create(const ComponentDesc *pType, Kernel *pKernel, udRCString uid, udInitParams initParams)
{
  return nullptr;
}

} // namespace ud

#endif
