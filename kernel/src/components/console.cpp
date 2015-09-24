#include "components/console.h"

namespace ep
{

ComponentDesc Console::descriptor =
{
  &Stream::descriptor, // pSuperDesc

  UDSHELL_APIVERSION, // udVersion
  UDSHELL_PLUGINVERSION, // pluginVersion

  "console", // id
  "Console", // displayName
  "Standard in/out console", // description
};

Console::Console(const ComponentDesc *pType, Kernel *pKernel, epSharedString uid, epInitParams initParams)
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

epSlice<void> Console::Read(epSlice<void> buffer)
{
  size_t read = fread(buffer.ptr, 1, buffer.length, pIn);
  return buffer.slice(0, read);
}

size_t Console::Write(epSlice<const void> data)
{
  if (bDbgOutput)
  {
#if defined(EP_WINDOWS)
    epString s((const char*)data.ptr, data.length);
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

} // namespace ep
