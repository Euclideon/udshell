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


// ---------------------------------------------------------------------------------------
// Author: Manu Evans, May 2015
udResult udKernel_RunMainLoop(udKernel *pKernel)
{
  // block and return when done...
  return udR_Success;
}

// ---------------------------------------------------------------------------------------
// Author: Manu Evans, May 2015
udResult udKernel_Quit()
{
  // cause a quit
  return udR_Success;
}

#endif
