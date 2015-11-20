#include "components/console.h"

namespace kernel
{

Console::Console(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, InitParams initParams)
  : Stream(pType, pKernel, uid, initParams)
{
  bDbgOutput = false;

  ConsoleOutputs out = initParams["output"].as<ConsoleOutputs>();

  pIn = stdin;

  if (out == ConsoleOutputs::StdDbg)
  {
    pOut = stderr;
#if UD_DEBUG && defined(EP_WINDOWS)
    if (IsDebuggerPresent())
    {
      pOut = nullptr;
      bDbgOutput = true;
    }
#endif
  }
  else if (out == ConsoleOutputs::StdErr)
    pOut = stderr;
  else
    pOut = stdout;
}

Slice<void> Console::Read(Slice<void> buffer)
{
  size_t read = fread(buffer.ptr, 1, buffer.length, pIn);
  return buffer.slice(0, read);
}

size_t Console::Write(Slice<const void> data)
{
  if (bDbgOutput)
  {
#if defined(EP_WINDOWS)
    String s((const char*)data.ptr, data.length);
    OutputDebugString((LPCSTR)s.toStringz());
#endif
    return data.length;
  }
  else
    return fwrite(data.ptr, 1, data.length, pOut);
}

int Console::Flush()
{
  if (pOut)
  {
    if(fflush(pOut))
      return -1;
  }

  return 0;
}

} // namespace kernel
