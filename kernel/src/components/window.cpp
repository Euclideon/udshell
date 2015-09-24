#include "window.h"
#include "ui.h"

namespace ep
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

  epSlice<CPropertyDesc>(windowProps, UDARRAYSIZE(windowProps)), // properties
  nullptr,
  nullptr
};


// ---------------------------------------------------------------------------------------
Window::Window(const ComponentDesc *pType, Kernel *pKernel, epSharedString uid, epInitParams initParams)
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

} // namespace ep
