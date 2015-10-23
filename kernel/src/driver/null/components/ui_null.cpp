#include "hal/driver.h"

#if EPUI_DRIVER == EPDRIVER_NULL

#include "components/ui.h"
#include "components/viewport.h"
#include "components/window.h"

namespace ep
{

Variant UIComponent::GetUIHandle() const
{
  return Variant();
}

epResult UIComponent::CreateInternal(InitParams epUnusedParam(initParams))
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
epResult Viewport::CreateInternal(InitParams epUnusedParam(initParams))
{
  return epR_Success;
}

// ---------------------------------------------------------------------------------------
void Viewport::DestroyInternal()
{
}


// ---------------------------------------------------------------------------------------
epResult Window::CreateInternal(InitParams epUnusedParam(initParams))
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

#else
EPEMPTYFILE
#endif
