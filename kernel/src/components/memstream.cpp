#include "components/memstream.h"
#include "components/broadcaster.h"
#include "kernel.h"

namespace kernel {

MemStream::MemStream(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
  : Stream(pType, pKernel, uid, initParams)
{
  BufferRef inBuffer;

  const Variant *fl = initParams.Get("flags");
  oFlags = fl->as<OpenFlags>();
  if (!(oFlags & (OpenFlags::Write | OpenFlags::Read)))
    throw epR_InvalidParameter;

  const Variant *buf = initParams.Get("buffer");
  if (!buf->isValid())
  {
    inBuffer = pKernel->CreateComponent<Buffer>();
    inBuffer->Reserve(DefaultBufferSize);
  }
  else
  {
    inBuffer = buf->as<BufferRef>();
    if (!inBuffer)
      throw epR_InvalidParameter;
  }

  SetBuffer(inBuffer);
}

MemStream::~MemStream()
{
  if (spBuffer)
    spBuffer->Unmap();
}

BufferRef MemStream::GetBuffer() const
{
  return spBuffer;
}

void MemStream::SetBuffer(BufferRef spNewBuffer)
{
  if (spBuffer)
  {
    spBuffer->Unmap();
    pos = 0;
  }

  length = 0;
  spBuffer = spNewBuffer;

  if (!spBuffer)
    return;

  if (oFlags & OpenFlags::Write)
  {
    bufferSlice = spBuffer->Map();
    if (bufferSlice == nullptr)
      LogError("Can't reserve Buffer for writing.");
  }
  else if (oFlags & OpenFlags::Read)
  {
    auto map = spBuffer->MapForRead();
    bufferSlice = Slice<void>(const_cast<void*>(map.ptr), map.length);
    if (!bufferSlice)
      LogError("Can't reserve Buffer for reading.");
  }

  if (bufferSlice == nullptr)
  {
    spBuffer = nullptr;
    return; // TODO Error handling
  }

  length = bufferSlice.length;
}

Slice<void> MemStream::Read(Slice<void> buf)
{
  if (!spBuffer)
    return 0;

  size_t bytes = buf.length;
  if (pos + bytes > (size_t)length)
    bytes = size_t(length - pos);

  memcpy(buf.ptr, (const char*)bufferSlice.ptr + pos, bytes);
  pos += bytes;

  return Slice<void>(buf.ptr, bytes);
}

size_t MemStream::Write(Slice<const void> data)
{
  if (!(oFlags & OpenFlags::Write) || !spBuffer)
    return 0;

  if (pos + data.length > (size_t)length)
  {
    spBuffer->Unmap();
    spBuffer->Resize(size_t(pos + data.length));

    bufferSlice = spBuffer->Map();
    length = bufferSlice.length;
  }

  memcpy((char*)bufferSlice.ptr + pos, data.ptr, data.length);
  pos += data.length;

  Broadcaster::Write(data);

  return data.length;
}

int64_t MemStream::Seek(SeekOrigin rel, int64_t offset)
{
  switch (rel)
  {
    case SeekOrigin::Begin:
      pos = offset;
      break;
    case SeekOrigin::CurrentPos:
      pos += offset;
      break;
    case SeekOrigin::End:
      pos = length + offset;
      break;
  }

  if (pos > length)
    pos = length;

  PosChanged.Signal();

  return pos;
}

} // namespace kernel
