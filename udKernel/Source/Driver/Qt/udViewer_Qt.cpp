#include "udDriver.h"

#if UDINPUT_DRIVER == UDDRIVER_QT

#include "udKernel.h"


udKernel *udKernel::CreateInstanceInternal(udInitParams commandLine)
{
  return new udKernel;
}

udResult udKernel::InitInstanceInternal()
{
  // TODO: *** MOVE THIS TO RENDER THREAD!! ***
  return InitRender();
}

udResult udKernel::DestroyInstanceInternal()
{
  delete this;
  return udR_Success;
}

udViewRef udKernel::SetFocusView(udViewRef pView)
{
  udViewRef pOld = pFocusView;
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
