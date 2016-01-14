#include "hal/driver.h"

#if EPUI_DRIVER == EPDRIVER_NULL

#include "components/uicomponentimpl.h"
#include "components/viewport.h"
#include "components/window.h"

namespace ep
{

Variant UIComponentImpl::GetUIHandle() const
{
  return Variant();
}

epResult UIComponentImpl::CreateInternal(Variant::VarMap epUnusedParam(initParams))
{
  return epR_Success;
}

// ---------------------------------------------------------------------------------------
epResult UIComponentImpl::InitComplete()
{
  return epR_Success;
}

// ---------------------------------------------------------------------------------------
void UIComponentImpl::DestroyInternal()
{
}


// ---------------------------------------------------------------------------------------
epResult Viewport::CreateInternal(Variant::VarMap epUnusedParam(initParams))
{
  return epR_Success;
}

// ---------------------------------------------------------------------------------------
void Viewport::DestroyInternal()
{
}


// ---------------------------------------------------------------------------------------
epResult Window::CreateInternal(Variant::VarMap epUnusedParam(initParams))
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
