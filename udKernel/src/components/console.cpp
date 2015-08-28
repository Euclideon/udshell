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

Console::Console(const ComponentDesc *pType, Kernel *pKernel, udRCString uid, udInitParams initParams)
  : Stream(pType, pKernel, uid, initParams)
{
  bDbgOutput = false;

  ConsoleOutputs out = initParams["output"].as<ConsoleOutputs>();

  pIn = stdin;

  if (out == ConsoleOutputs::StdDbg)
  {
    if (UD_DEBUG && UDPLATFORM_WINDOWS && IsDebuggerPresent())
    {
      pOut = nullptr;
      bDbgOutput = true;
    }
    else
      pOut = stderr;
  }
  else if (out == ConsoleOutputs::StdOut)
    pOut = stdout;
  else
    pOut = stderr;
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
    OutputDebugString((LPCSTR)pData);
    written = bytes;
  }
  else
    written = fwrite(pData, 1, bytes, pOut);

  return written;
}

size_t Console::WriteLn(const char *pData)
{
  fputs(pData, pOut);

  return strlen(pData);
}

size_t Console::ReadLn(char *pData, size_t bytes)
{
  size_t read;

  fgets(pData, (int)bytes, pOut);

  read = strlen(pData) - 1;
  pData[read] = '\0';

  return read;
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
