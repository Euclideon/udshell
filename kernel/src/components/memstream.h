#pragma once
#if !defined(_EP_MEMSTREAM_H)
#define _EP_MEMSTREAM_H

#include "ep/cpp/component/stream.h"

namespace ep
{
SHARED_CLASS(MemStream);
SHARED_CLASS(Buffer);

class MemStream : public Stream
{
  EP_DECLARE_COMPONENT(MemStream, Stream, EPKERNEL_PLUGINVERSION, "Memory stream", 0)
public:

  Slice<void> Read(Slice<void> buffer) override;
  size_t Write(Slice<const void> data) override;

  int64_t Seek(SeekOrigin rel, int64_t offset) override;

  BufferRef GetBuffer() const;

protected:
  MemStream(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams);
  ~MemStream();

  void SetBuffer(BufferRef spNewBuffer);

  static const size_t DefaultBufferSize = 102400;

  BufferRef spBuffer;
  Slice<void> bufferSlice;
  OpenFlags oFlags;

  Array<const PropertyInfo> GetProperties() const;
};

}

#endif // _EP_MEMSTREAM_H
