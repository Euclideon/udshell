#pragma once
#if !defined(_UD_CONSOLE_H)
#define _UD_CONSOLE_H

#include "stream.h"

UD_ENUM(ConsoleOutputs,
  StdOut,
  StdErr,
  StdDbg
);

namespace ud
{

PROTOTYPE_COMPONENT(Console);

class Console : public Stream
{
public:
  UD_COMPONENT(Console);

  udSlice<void> Read(udSlice<void> buffer) override;
  size_t Write(udSlice<const void> data) override;
  int Flush() override;

protected:
  Console(const ComponentDesc *pType, Kernel *pKernel, udSharedString uid, udInitParams initParams);

  FILE *pIn, *pOut;
  bool bDbgOutput;
};

}

#endif // _UD_CONSOLE_H
