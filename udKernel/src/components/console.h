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

  size_t Read(void *pData, size_t bytes) override;
  size_t Write(const void *pData, size_t bytes) override;
  int Flush() override;

  size_t WriteLn(udString str);
  udString ReadLn(udSlice<char> buf);

protected:
  Console(const ComponentDesc *pType, Kernel *pKernel, udRCString uid, udInitParams initParams);

  FILE *pIn, *pOut;
  bool bDbgOutput;
};

}

#endif // _UD_CONSOLE_H
