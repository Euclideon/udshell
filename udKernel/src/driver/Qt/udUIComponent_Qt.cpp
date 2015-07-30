#include "hal/driver.h"

#if UDUI_DRIVER == UDDRIVER_QT

#include "ui/qtuicomponent.h"

namespace ud
{

// ---------------------------------------------------------------------------------------
Component *UIComponent::Create(const ComponentDesc *pType, Kernel *pKernel, udRCString uid, InitParams initParams)
{
  return udNew(qt::UIComponent, pType, pKernel, uid, initParams);
}

} // namespace ud

#endif
