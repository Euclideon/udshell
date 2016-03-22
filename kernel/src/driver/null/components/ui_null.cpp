#include "hal/driver.h"

#if EPUI_DRIVER == EPDRIVER_NULL

#include "components/uicomponentimpl.h"
#include "components/viewportimpl.h"
#include "components/windowimpl.h"

namespace ep
{

Variant UIComponentImpl::GetUIHandle() const
{
  return Variant();
}

void UIComponentImpl::CreateInternal(Variant::VarMap epUnusedParam(initParams))
{
}

// ---------------------------------------------------------------------------------------
void UIComponentImpl::DestroyInternal()
{
}


// ---------------------------------------------------------------------------------------
void ViewportImpl::CreateInternal(Variant::VarMap epUnusedParam(initParams))
{
}


// ---------------------------------------------------------------------------------------
void WindowImpl::CreateInternal(Variant::VarMap epUnusedParam(initParams))
{
}

// ---------------------------------------------------------------------------------------
void WindowImpl::DestroyInternal()
{
}

// ---------------------------------------------------------------------------------------
void WindowImpl::SetTopLevelUI(UIComponentRef epUnusedParam(spUIComponent))
{
}

} // namespace ep

#else
EPEMPTYFILE
#endif
