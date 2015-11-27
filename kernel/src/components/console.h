#pragma once
#if !defined(_EP_CONSOLE_H)
#define _EP_CONSOLE_H

#include "stream.h"

EP_ENUM(ConsoleOutputs,
  StdOut,
  StdErr,
  StdDbg
);

namespace kernel
{

PROTOTYPE_COMPONENT(Console);

class Console : public Stream
{
  EP_DECLARE_COMPONENT(Console, Stream, EPKERNEL_PLUGINVERSION, "Standard in/out console")
public:

  Slice<void> Read(Slice<void> buffer) override;
  size_t Write(Slice<const void> data) override;
  int Flush() override;

protected:
  Console(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, InitParams initParams);

  FILE *pIn, *pOut;
  bool bDbgOutput;
};

}

#endif // _EP_CONSOLE_H
