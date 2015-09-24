#include "hal/driver.h"

#if EPWINDOW_DRIVER == EPDRIVER_NULL

#include "kernel.h"

namespace ep
{

class NullKernel : public Kernel
{
public:
  NullKernel() {}
  udResult InitInternal() { return udR_Success; }
};

Kernel *Kernel::CreateInstanceInternal(epInitParams commandLine)
{
  return new NullKernel;
}

// ---------------------------------------------------------------------------------------
// Author: Manu Evans, May 2015
udResult Kernel::RunMainLoop()
{
  return udR_Success;
}

// ---------------------------------------------------------------------------------------
// Author: Manu Evans, May 2015
udResult Kernel::Terminate()
{
  return udR_Success;
}

void Kernel::DispatchToMainThread(MainThreadCallback callback)
{

}

void Kernel::DispatchToMainThreadAndWait(MainThreadCallback callback)
{

}

}

#endif
