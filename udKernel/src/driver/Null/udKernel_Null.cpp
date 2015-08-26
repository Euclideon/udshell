#include "hal/driver.h"

#if UDWINDOW_DRIVER == UDDRIVER_NULL

#include "kernel.h"


Kernel *Kernel::createInstanceInternal(int argc, char** argv)
{
  return new Kernel;
}

udResult Kernel::destroyInstanceInternal()
{
  delete this;
  return udR_Success;
}


// ---------------------------------------------------------------------------------------
// Author: Manu Evans, May 2015
udResult udKernel_RunMainLoop(Kernel *pKernel)
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
