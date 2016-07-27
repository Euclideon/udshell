#pragma once
#ifndef EPMEMSTREAM_H
#define EPMEMSTREAM_H

#include "ep/cpp/internal/i/imemstream.h"

#include "ep/cpp/component/stream.h"

namespace ep
{
SHARED_CLASS(MemStream);
SHARED_CLASS(Buffer);

class MemStream : public Stream
{
  EP_DECLARE_COMPONENT_WITH_IMPL(ep, MemStream, IMemStream, Stream, EPKERNEL_PLUGINVERSION, "Memory stream", 0)
public:

  Slice<void> Read(Slice<void> buffer) override { return pImpl->Read(buffer); }
  size_t Write(Slice<const void> data) override { return pImpl->Write(data); }

  int64_t Seek(SeekOrigin rel, int64_t offset) override { return pImpl->Seek(rel, offset); }

  virtual BufferRef GetBuffer() const { return pImpl->GetBuffer(); }

protected:
  MemStream(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
    : Stream(pType, pKernel, uid, initParams)
  {
    pImpl = createImpl(initParams);
  }

private:
  Array<const PropertyInfo> getProperties() const;
};

}

#endif // EPMEMSTREAM_H
