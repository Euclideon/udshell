#pragma once
#if !defined(_EP_MEMSTREAM_H)
#define _EP_MEMSTREAM_H

#include "stream.h"

namespace kernel
{
PROTOTYPE_COMPONENT(MemStream);
SHARED_CLASS(Buffer);

class MemStream : public Stream
{
  EP_DECLARE_COMPONENT(MemStream, Stream, EPKERNEL_PLUGINVERSION, "Memory stream")
public:

  Slice<void> Read(Slice<void> buffer) override;
  size_t Write(Slice<const void> data) override;

  int64_t Seek(SeekOrigin rel, int64_t offset) override;

  BufferRef GetBuffer() const;
  void SetBuffer(BufferRef spNewBuffer);

  Event<> Changed;
protected:
  MemStream(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, InitParams initParams);
  ~MemStream();

  static const size_t DefaultBufferSize = 102400;

  BufferRef spBuffer;
  Slice<void> bufferSlice;
  OpenFlags oFlags;

  static Array<const PropertyInfo> GetProperties()
  {
    return{
      EP_MAKE_PROPERTY(Buffer, "The Buffer component MemStream reads/writes to", nullptr, 0),
    };
  }
};

}

#endif // _EP_MEMSTREAM_H
