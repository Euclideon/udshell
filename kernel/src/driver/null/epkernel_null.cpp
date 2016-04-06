#include "hal/driver.h"

#if EPWINDOW_DRIVER == EPDRIVER_NULL

#include "ep/cpp/kernel.h"

namespace ep
{

class NullKernel : public Kernel
{
public:
  NullKernel() {}

  void InitInternal() override { }
};

Kernel *Kernel::CreateInstanceInternal(Slice<const KeyValuePair> commandLine)
{
  return epNew(NullKernel);
}

// ---------------------------------------------------------------------------------------
// Author: Manu Evans, May 2015
void Kernel::Quit()
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

#else
EPEMPTYFILE
#endif
