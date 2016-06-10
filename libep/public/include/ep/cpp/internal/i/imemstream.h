#pragma once
#if !defined(_EP_IMEMSTREAM_HPP)
#define _EP_IMEMSTREAM_HPP

#include "ep/cpp/component/stream.h"
#include "ep/cpp/component/component.h"

namespace ep
{
SHARED_CLASS(MemStream);
SHARED_CLASS(Buffer);

class IMemStream
{
public:
  virtual Slice<void> Read(Slice<void> buffer) = 0;
  virtual size_t Write(Slice<const void> data) = 0;

  virtual int64_t Seek(SeekOrigin rel, int64_t offset) = 0;

  virtual BufferRef GetBuffer() const = 0;
};

}

#endif // _EP_IMEMSTREAM_HPP
