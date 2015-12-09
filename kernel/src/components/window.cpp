#include "window.h"
#include "ui.h"

namespace ep {

// ---------------------------------------------------------------------------------------
Window::Window(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
  : Component(pType, pKernel, uid, initParams)
{
  if (CreateInternal(initParams) != epR_Success)
    throw epR_Failure;
}

// ---------------------------------------------------------------------------------------
Window::~Window()
{
  DestroyInternal();
}

} // namespace ep
