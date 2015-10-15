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

  EPSHELL_APIVERSION,     // epVersion
  EPSHELL_PLUGINVERSION,  // pluginVersion

  "window",               // id
  "Window",               // displayName
  "Is a window",          // description

  Slice<CPropertyDesc>(windowProps, UDARRAYSIZE(windowProps)), // properties
  nullptr,
  nullptr
};


// ---------------------------------------------------------------------------------------
Window::Window(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, InitParams initParams)
  : Component(pType, pKernel, uid, initParams)
{
  if (CreateInternal(initParams) != epR_Success)
    throw epR_Failure_;
}

// ---------------------------------------------------------------------------------------
Window::~Window()
{
  DestroyInternal();
}

} // namespace ep
