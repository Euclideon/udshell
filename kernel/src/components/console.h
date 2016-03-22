#pragma once
#if !defined(_EP_CONSOLE_H)
#define _EP_CONSOLE_H

#include "ep/cpp/component/stream.h"

EP_ENUM(ConsoleOutputs,
  StdOut,
  StdErr,
  StdDbg
);

namespace ep
{

SHARED_CLASS(Console);

class Console : public Stream
{
  EP_DECLARE_COMPONENT(Console, Stream, EPKERNEL_PLUGINVERSION, "Standard in/out console", 0)
public:

  Slice<void> Read(Slice<void> buffer) override;
  size_t Write(Slice<const void> data) override;
  int Flush() override;

protected:
  Console(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams);

  FILE *pIn, *pOut;
  bool bDbgOutput;
};

}

#endif // _EP_CONSOLE_H
