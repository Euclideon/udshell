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
  EP_DECLARE_COMPONENT(ep, StdIOStream, Stream, EPKERNEL_PLUGINVERSION, "Standard in/out stream", 0)
public:

  Slice<void> read(Slice<void> buffer) override;
  size_t write(Slice<const void> data) override;
  int flush() override;

protected:
  StdIOStream(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams);

  FILE *pIn, *pOut;
  bool bDbgOutput;
};

}

#endif // _EP_STDIOSTREAM_H
