#include "components/console.h"

namespace ud
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

Console::Console(const ComponentDesc *pType, Kernel *pKernel, udSharedString uid, udInitParams initParams)
  : Stream(pType, pKernel, uid, initParams)
{
  bDbgOutput = false;

  ConsoleOutputs out = initParams["output"].as<ConsoleOutputs>();

  pIn = stdin;

  if (out == ConsoleOutputs::StdDbg)
  {
    pOut = stderr;
#if UD_DEBUG && UDPLATFORM_WINDOWS
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

size_t Console::Read(void *pData, size_t bytes)
{
  size_t read;

  read = fread(pData, 1, bytes, pIn);

  return read;
}

size_t Console::Write(const void *pData, size_t bytes)
{
  size_t written;

  if (bDbgOutput)
  {
#if UDPLATFORM_WINDOWS
    OutputDebugString((LPCSTR)pData);
#endif
    written = bytes;
  }
  else
    written = fwrite(pData, 1, bytes, pOut);

  return written;
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

} // namespace ud
