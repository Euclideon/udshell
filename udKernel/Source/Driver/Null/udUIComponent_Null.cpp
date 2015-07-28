#include "udDriver.h"

#if UDUI_DRIVER == UDDRIVER_NULL

#include "udUIComponent.h"

namespace udKernel
{

Component *UIComponent::Create(const ComponentDesc *pType, Kernel *pKernel, udRCString uid, InitParams initParams)
{
  return nullptr;
}

} // namespace udKernel

#endif
