#pragma once
#if !defined(_EP_CONSOLE_H)
#define _EP_CONSOLE_H

#include "stream.h"

UD_ENUM(ConsoleOutputs,
  StdOut,
  StdErr,
  StdDbg
);

namespace ep
{

PROTOTYPE_COMPONENT(Console);

class Console : public Stream
{
public:
  EP_COMPONENT(Console);

  epSlice<void> Read(epSlice<void> buffer) override;
  size_t Write(epSlice<const void> data) override;
  int Flush() override;

protected:
  Console(const ComponentDesc *pType, Kernel *pKernel, epSharedString uid, epInitParams initParams);

  FILE *pIn, *pOut;
  bool bDbgOutput;
};

}

#endif // _EP_CONSOLE_H
