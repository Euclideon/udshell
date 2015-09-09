#include "window.h"
#include "ui.h"

namespace ud
{

static CPropertyDesc windowProps[] =
{
  {
    {
      "ui", // id
      "UI", // displayName
      "Top level UI for the window", // description
    },
    &Window::GetUI, // getter
    &Window::SetUI, // setter
  }
};

ComponentDesc Window::descriptor =
{
  &Component::descriptor, // pSuperDesc

  UDSHELL_APIVERSION,     // udVersion
  UDSHELL_PLUGINVERSION,  // pluginVersion

  "window",               // id
  "Window",               // displayName
  "Is a window",          // description
};


// ---------------------------------------------------------------------------------------
Window::Window(const ComponentDesc *pType, Kernel *pKernel, udSharedString uid, udInitParams initParams)
  : Component(pType, pKernel, uid, initParams)
{
  if (CreateInternal(initParams) != udR_Success)
    throw udR_Failure_;
}

// ---------------------------------------------------------------------------------------
Window::~Window()
{
  DestroyInternal();
}

// ---------------------------------------------------------------------------------------
void Window::SetUI(UIComponentRef spUIComponent)
{
  spTopLevelUI = spUIComponent;
  // ... DO SOMETHING...
}

} // namespace ud
