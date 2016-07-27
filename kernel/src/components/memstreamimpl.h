#pragma once
#ifndef EPMEMSTREAMIMPL_H
#define EPMEMSTREAMIMPL_H

#include "ep/cpp/component/memstream.h"
#include "ep/cpp/internal/i/imemstream.h"

#include "ep/cpp/component/stream.h"

namespace ep
{
SHARED_CLASS(MemStream);
SHARED_CLASS(Buffer);

class MemStreamImpl : public BaseImpl<MemStream, IMemStream>
{
public:
  MemStreamImpl(Component *pInstance, Variant::VarMap initParams);

  Slice<void> Read(Slice<void> buffer) override final;
  size_t Write(Slice<const void> data) override final;

  int64_t Seek(SeekOrigin rel, int64_t offset) override final;

  BufferRef GetBuffer() const override final;

protected:
  ~MemStreamImpl();

  void SetBuffer(BufferRef spNewBuffer);

  static const size_t DefaultBufferSize = 102400;

  BufferRef spBuffer;
  Slice<void> bufferSlice;
  OpenFlags oFlags;

  Array<const PropertyInfo> getProperties() const;
};

}

#endif // EPMEMSTREAMIMPL_H
