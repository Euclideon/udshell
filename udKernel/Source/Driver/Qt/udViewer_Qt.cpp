#include "udDriver.h"

#if UDINPUT_DRIVER == UDDRIVER_QT

#include "udKernel.h"


udKernel *udKernel::CreateInstanceInternal(udInitParams commandLine)
{
  return new udKernel;
}

udResult udKernel::DestroyInstanceInternal()
{
  delete this;
  return udR_Success;
}

udView *udKernel::SetFocusView(udView *pView)
{
  udView *pOld = pFocusView;
  pFocusView = pView;
  return pOld;
}

udResult udKernel::RunMainLoop()
{
  // TODO: should we execute the Qt main loop here or leave it to the frontend?

  return udR_Success;
}

udResult udKernel::Terminate()
{
  return udR_Success;
}

#endif
