#include "hal/driver.h"

#if UDUI_DRIVER == UDDRIVER_QT

#include "ui/window.h"

namespace ud
{

udResult UIComponent::CreateInternal(udString filename)
{
  try
  {
    pInternal = new qt::QtComponent(this, filename);

    // We expect a QQuickItem object
    // TODO: better error handling?
    UDASSERT(qobject_cast<QQuickItem*>(pInternal->pQtObject) != nullptr, "Qt UIComponent must create a QQuickItem");
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


udResult Viewport::CreateInternal()
{
  return udR_Success;
}

void Viewport::DestroyInternal()
{
}


udResult Window::CreateInternal(udString filename)
{
  try
  {
    pInternal = new qt::QtWindow(this, filename);
  }
  catch (...)
  {
    return udR_Failure_;
  }
  return udR_Success;
}

void Window::DestroyInternal()
{
  delete pInternal;
  pInternal = nullptr;
}

void Window::Refresh()
{
  pInternal->Refresh();
}

} // namespace ud

#endif
