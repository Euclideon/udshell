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
    &Window::GetTopLevelUI, // getter
    &Window::SetTopLevelUI, // setter
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

  udSlice<CPropertyDesc>(windowProps, UDARRAYSIZE(windowProps)), // properties
  nullptr,
  nullptr
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

} // namespace ud
