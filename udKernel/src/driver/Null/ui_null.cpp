#include "hal/driver.h"

#if UDUI_DRIVER == UDDRIVER_NULL

#include "components/ui.h"
#include "components/viewport.h"
#include "components/window.h"

namespace ud
{

udResult UIComponent::CreateInternal(udInitParams udUnusedParam(initParams))
{
  return udR_Success;
}

// ---------------------------------------------------------------------------------------
void UIComponent::DestroyInternal()
{
}


// ---------------------------------------------------------------------------------------
udResult Viewport::CreateInternal(udInitParams udUnusedParam(initParams))
{
  return udR_Success;
}

// ---------------------------------------------------------------------------------------
void Viewport::DestroyInternal()
{
}


// ---------------------------------------------------------------------------------------
udResult Window::CreateInternal(udInitParams udUnusedParam(initParams))
{
  return udR_Success;
}

// ---------------------------------------------------------------------------------------
void Window::DestroyInternal()
{
}

// ---------------------------------------------------------------------------------------
void Window::SetTopLevelUI(UIComponentRef udUnusedParam(spUIComponent))
{
}

} // namespace ud

#endif
