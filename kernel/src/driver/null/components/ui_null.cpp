#include "hal/driver.h"

#if EPUI_DRIVER == EPDRIVER_NULL

#include "components/ui.h"
#include "components/viewport.h"
#include "components/window.h"

namespace ep
{

epResult UIComponent::CreateInternal(epInitParams epUnusedParam(initParams))
{
  return epR_Success;
}

// ---------------------------------------------------------------------------------------
epResult UIComponent::InitComplete()
{
  return epR_Success;
}

// ---------------------------------------------------------------------------------------
void UIComponent::DestroyInternal()
{
}


// ---------------------------------------------------------------------------------------
epResult Viewport::CreateInternal(epInitParams epUnusedParam(initParams))
{
  return epR_Success;
}

// ---------------------------------------------------------------------------------------
void Viewport::DestroyInternal()
{
}


// ---------------------------------------------------------------------------------------
epResult Window::CreateInternal(epInitParams epUnusedParam(initParams))
{
  return epR_Success;
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
