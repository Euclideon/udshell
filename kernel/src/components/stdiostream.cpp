#include "components/stdiostream.h"
#include "ep/cpp/component/broadcaster.h"

namespace ep
{

StdIOStream::StdIOStream(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
  : Stream(pType, pKernel, uid, initParams)
{
  bDbgOutput = false;

  StdIOStreamOutputs out = initParams.get("output")->as<StdIOStreamOutputs>();

  pIn = stdin;

  if (out == StdIOStreamOutputs::StdDbg)
  {
    pOut = stderr;
#if defined(EP_WINDOWS)
    if (IsDebuggerPresent())
    {
      pOut = nullptr;
      bDbgOutput = true;
    }
#endif
  }
  else if (out == StdIOStreamOutputs::StdErr)
    pOut = stderr;
  else
    pOut = stdout;
}

Slice<void> StdIOStream::Read(Slice<void> buffer)
{
  size_t read = fread(buffer.ptr, 1, buffer.length, pIn);
  return buffer.slice(0, read);
}

size_t StdIOStream::Write(Slice<const void> data)
{
  size_t written;

  if (bDbgOutput)
  {
#if defined(EP_WINDOWS)
    Array<wchar_t, 128> buffer(Alloc, data.length + 1);
    int len = MultiByteToWideChar(CP_UTF8, 0, (char*)data.ptr, (int)data.length, buffer.ptr, (int)data.length);
    if (len > 0)
    {
      buffer.ptr[len] = 0;
      OutputDebugStringW(buffer.ptr);
    }
#endif
    written = data.length;
  }
  else
    written = fwrite(data.ptr, 1, data.length, pOut);

  Broadcaster::Write(data);

  return written;
}

int StdIOStream::Flush()
{
  if (pOut)
  {
    if(fflush(pOut))
      return -1;
  }

  return 0;
}

} // namespace ep
