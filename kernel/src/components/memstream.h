#pragma once
#if !defined(_UD_MEMSTREAM_H)
#define _UD_MEMSTREAM_H

#include "stream.h"

namespace ud
{
PROTOTYPE_COMPONENT(MemStream);
SHARED_CLASS(Buffer);

class MemStream : public Stream
{
public:
  UD_COMPONENT(MemStream);

  udSlice<void> Read(udSlice<void> buffer) override;
  size_t Write(udSlice<const void> data) override;

  int64_t Seek(SeekOrigin rel, int64_t offset) override;

  BufferRef GetBuffer() const;
  void SetBuffer(BufferRef spNewBuffer);

protected:
  MemStream(const ComponentDesc *pType, Kernel *pKernel, udSharedString uid, udInitParams initParams);
  ~MemStream();

  static const size_t DefaultBufferSize = 102400;

  BufferRef spBuffer;
  udSlice<void> buffer;
  OpenFlags oFlags;
};

}

#endif // _UD_MEMSTREAM_H
