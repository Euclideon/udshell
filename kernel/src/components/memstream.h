#pragma once
#if !defined(_EP_MEMSTREAM_H)
#define _EP_MEMSTREAM_H

#include "stream.h"

namespace ep
{
PROTOTYPE_COMPONENT(MemStream);
SHARED_CLASS(Buffer);

class MemStream : public Stream
{
public:
  EP_COMPONENT(MemStream);

  Slice<void> Read(Slice<void> buffer) override;
  size_t Write(Slice<const void> data) override;

  int64_t Seek(SeekOrigin rel, int64_t offset) override;

  BufferRef GetBuffer() const;
  void SetBuffer(BufferRef spNewBuffer);

  epEvent<> Changed;
protected:
  MemStream(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, InitParams initParams);
  ~MemStream();

  static const size_t DefaultBufferSize = 102400;

  BufferRef spBuffer;
  Slice<void> bufferSlice;
  OpenFlags oFlags;
};

}

#endif // _EP_MEMSTREAM_H
