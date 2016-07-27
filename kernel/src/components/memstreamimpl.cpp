#include "components/memstreamimpl.h"
#include "ep/cpp/component/broadcaster.h"
#include "ep/cpp/kernel.h"

namespace ep {

Array<const PropertyInfo> MemStream::getProperties() const
{
  return{
    EP_MAKE_PROPERTY_RO("buffer", GetBuffer, "The Buffer component MemStream reads/writes to", nullptr, 0),
  };
}

MemStreamImpl::MemStreamImpl(Component *pInstance, Variant::VarMap initParams)
  : ImplSuper(pInstance)
{
  BufferRef inBuffer;

  const Variant *fl = initParams.get("flags");
  if (!fl)
    EPTHROW_ERROR(Result::InvalidArgument, "Missing 'flags'");

  oFlags = fl->as<OpenFlags>();
  if (!(oFlags & (OpenFlags::Write | OpenFlags::Read)))
    EPTHROW_ERROR(Result::InvalidArgument, "Invalid 'flags'");

  const Variant *buf = initParams.get("buffer");
  if (!buf)
  {
    inBuffer = GetKernel()->CreateComponent<Buffer>();
    inBuffer->Reserve(DefaultBufferSize);
  }
  else
  {
    inBuffer = buf->as<BufferRef>();
    if (!inBuffer)
      EPTHROW_ERROR(Result::InvalidArgument, "Expected 'buffer' is Buffer object");
  }

  SetBuffer(inBuffer);
}

MemStreamImpl::~MemStreamImpl()
{
  if (spBuffer)
    spBuffer->Unmap();
}

BufferRef MemStreamImpl::GetBuffer() const
{
  return spBuffer;
}

void MemStreamImpl::SetBuffer(BufferRef spNewBuffer)
{
  if (spBuffer)
  {
    spBuffer->Unmap();
    pInstance->Stream::SetPos(0);
  }

  pInstance->Stream::SetLength(0);
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

  pInstance->Stream::SetLength(bufferSlice.length);
}

Slice<void> MemStreamImpl::Read(Slice<void> buf)
{
  if (!spBuffer)
    return 0;

  size_t bytes = buf.length;
  int64_t pos = pInstance->Stream::GetPos();
  int64_t length = pInstance->Stream::Length();

  if (pos + bytes > (size_t)length)
    bytes = size_t(length - pos);

  memcpy(buf.ptr, (const char*)bufferSlice.ptr + pos, bytes);
  pInstance->Stream::SetPos(pos + bytes);

  return Slice<void>(buf.ptr, bytes);
}

size_t MemStreamImpl::Write(Slice<const void> data)
{
  if (!(oFlags & OpenFlags::Write) || !spBuffer)
    return 0;

  int64_t pos = pInstance->Stream::GetPos();
  int64_t length = pInstance->Stream::Length();

  if (pos + data.length > (size_t)length)
  {
    spBuffer->Unmap();
    spBuffer->Resize(size_t(pos + data.length));

    bufferSlice = spBuffer->Map();
    pInstance->Stream::SetLength(bufferSlice.length);
  }

  memcpy((char*)bufferSlice.ptr + pos, data.ptr, data.length);
  pInstance->Stream::SetPos(pos + data.length);

  pInstance->Broadcaster::Write(data);

  return data.length;
}

int64_t MemStreamImpl::Seek(SeekOrigin rel, int64_t offset)
{
  int64_t pos = pInstance->Stream::GetPos();
  int64_t length = pInstance->Stream::Length();

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

  pInstance->Stream::SetPos(pos);
  pInstance->Stream::PosChanged.Signal();

  return pos;
}

} // namespace ep
