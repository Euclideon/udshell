#pragma once
#if !defined(_EP_STDIOSTREAM_H)
#define _EP_STDIOSTREAM_H

#include "ep/cpp/component/stream.h"

namespace ep {

EP_ENUM(StdIOStreamOutputs,
  StdOut,
  StdErr,
  StdDbg
);

SHARED_CLASS(StdIOStream);

class StdIOStream : public Stream
{
  EP_DECLARE_COMPONENT(StdIOStream, Stream, EPKERNEL_PLUGINVERSION, "Standard in/out stream", 0)
public:

  Slice<void> Read(Slice<void> buffer) override;
  size_t Write(Slice<const void> data) override;
  int Flush() override;

protected:
  StdIOStream(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams);

  FILE *pIn, *pOut;
  bool bDbgOutput;
};

}

#endif // _EP_STDIOSTREAM_H
