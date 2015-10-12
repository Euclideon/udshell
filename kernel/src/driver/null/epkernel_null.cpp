#include "hal/driver.h"

#if EPWINDOW_DRIVER == EPDRIVER_NULL

#include "kernel.h"

namespace ep
{

class NullKernel : public Kernel
{
public:
  NullKernel() {}
  epResult InitInternal() { return epR_Success; }
};

Kernel *Kernel::CreateInstanceInternal(epInitParams commandLine)
{
  return new NullKernel;
}

// ---------------------------------------------------------------------------------------
// Author: Manu Evans, May 2015
epResult Kernel::RunMainLoop()
{
  return epR_Success;
}

// ---------------------------------------------------------------------------------------
// Author: Manu Evans, May 2015
epResult Kernel::Terminate()
{
  return epR_Success;
}

void Kernel::DispatchToMainThread(MainThreadCallback callback)
{

}

void Kernel::DispatchToMainThreadAndWait(MainThreadCallback callback)
{

}

}

#endif
