#include "udDriver.h"

#if UDINPUT_DRIVER == UDDRIVER_NULL

#include "udKernel.h"


udKernel *udKernel::createInstanceInternal(int argc, char** argv)
{
  return new udKernel;
}

udResult udKernel::destroyInstanceInternal()
{
  delete this;
  return udR_Success;
}


// ---------------------------------------------------------------------------------------
// Author: Manu Evans, May 2015
udResult udKernel_RunMainLoop(udKernel *pKernel)
{
  return udR_Success;
}

// ---------------------------------------------------------------------------------------
// Author: Manu Evans, May 2015
udResult udKernel_Quit()
{
  return udR_Success;
}

#endif
