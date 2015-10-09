#include "hal/driver.h"

#if EPUI_DRIVER == EPDRIVER_NULL

#include "components/ui.h"
#include "components/viewport.h"
#include "components/window.h"

namespace ep
{

udResult UIComponent::CreateInternal(epInitParams epUnusedParam(initParams))
{
  return udR_Success;
}

// ---------------------------------------------------------------------------------------
udResult UIComponent::InitComplete()
{
  return udR_Success;
}

// ---------------------------------------------------------------------------------------
void UIComponent::DestroyInternal()
{
}


// ---------------------------------------------------------------------------------------
udResult Viewport::CreateInternal(epInitParams epUnusedParam(initParams))
{
  return udR_Success;
}

// ---------------------------------------------------------------------------------------
void Viewport::DestroyInternal()
{
}


// ---------------------------------------------------------------------------------------
udResult Window::CreateInternal(epInitParams epUnusedParam(initParams))
{
  return udR_Success;
}

// ---------------------------------------------------------------------------------------
void Window::DestroyInternal()
{
}

// ---------------------------------------------------------------------------------------
void Window::SetTopLevelUI(UIComponentRef epUnusedParam(spUIComponent))
{
}

} // namespace ep

#endif
