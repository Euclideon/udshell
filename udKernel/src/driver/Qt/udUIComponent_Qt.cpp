#include "hal/driver.h"

#if UDUI_DRIVER == UDDRIVER_QT

#include "ui/qtuicomponent.h"

namespace ud
{

// ---------------------------------------------------------------------------------------
Component *UIComponent::Create(const ComponentDesc *pType, Kernel *pKernel, udRCString uid, udInitParams initParams)
{
  using namespace qt;
  return udNew(QtUIComponent, QtUIComponent::CreateComponentDesc(pType), pKernel, uid, initParams);
}

} // namespace ud

#endif
