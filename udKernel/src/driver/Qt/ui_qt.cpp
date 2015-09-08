#include "hal/driver.h"

#if UDUI_DRIVER == UDDRIVER_QT

#include "ui/uicomponent.h"
#include "ui/window.h"

namespace ud
{

udResult UIComponent::CreateInternal(udString filename)
{
  try
  {
    pInternal = new qt::QtComponent(this, filename);
  }
  catch (...)
  {
    return udR_Failure_;
  }
  return udR_Success;
}

void UIComponent::DestroyInternal()
{
  delete pInternal;
  pInternal = nullptr;
}

// ---------------------------------------------------------------------------------------
Component *Viewport::Create(const ComponentDesc *pType, Kernel *pKernel, udRCString uid, udInitParams initParams)
{
  using namespace qt;
  return udNew(QtViewport, QtViewport::CreateComponentDesc(pType), pKernel, uid, initParams);
}

// ---------------------------------------------------------------------------------------
Component *Window::Create(const ComponentDesc *pType, Kernel *pKernel, udRCString uid, udInitParams initParams)
{
  using namespace qt;
  return udNew(QtWindow, QtWindow::CreateComponentDesc(pType), pKernel, uid, initParams);
}

} // namespace ud

#endif
