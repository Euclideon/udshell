#include "components/memstreamimpl.h"
#include "ep/cpp/component/broadcaster.h"
#include "ep/cpp/kernel.h"

namespace ep {

Array<const PropertyInfo> MemStream::getProperties() const
{
  return{
    EP_MAKE_PROPERTY_RO("buffer", getBuffer, "The Buffer component MemStream reads/writes to", nullptr, 0),
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
    inBuffer = GetKernel()->createComponent<Buffer>();
    inBuffer->reserve(DefaultBufferSize);
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
    spBuffer->unmap();
}

BufferRef MemStreamImpl::GetBuffer() const
{
  return spBuffer;
}

void MemStreamImpl::SetBuffer(BufferRef spNewBuffer)
{
  if (spBuffer)
  {
    spBuffer->unmap();
    pInstance->Stream::setPos(0);
  }

  pInstance->Stream::setLength(0);
  spBuffer = spNewBuffer;

  if (!spBuffer)
    return;

  if (oFlags & OpenFlags::Write)
  {
    bufferSlice = spBuffer->map();
    if (bufferSlice == nullptr)
      LogError("Can't reserve Buffer for writing.");
  }
  else if (oFlags & OpenFlags::Read)
  {
    auto map = spBuffer->mapForRead();
    bufferSlice = Slice<void>(const_cast<void*>(map.ptr), map.length);
    if (!bufferSlice)
      LogError("Can't reserve Buffer for reading.");
  }

  if (bufferSlice == nullptr)
  {
    spBuffer = nullptr;
    return; // TODO Error handling
  }

  pInstance->Stream::setLength(bufferSlice.length);
}

Slice<void> MemStreamImpl::Read(Slice<void> buf)
{
  if (!spBuffer)
    return 0;

  size_t bytes = buf.length;
  int64_t pos = pInstance->Stream::getPos();
  int64_t length = pInstance->Stream::length();

  if (pos + bytes > (size_t)length)
    bytes = size_t(length - pos);

  memcpy(buf.ptr, (const char*)bufferSlice.ptr + pos, bytes);
  pInstance->Stream::setPos(pos + bytes);

  return Slice<void>(buf.ptr, bytes);
}

size_t MemStreamImpl::Write(Slice<const void> data)
{
  if (!(oFlags & OpenFlags::Write) || !spBuffer)
    return 0;

  int64_t pos = pInstance->Stream::getPos();
  int64_t length = pInstance->Stream::length();

  if (pos + data.length > (size_t)length)
  {
    spBuffer->unmap();
    spBuffer->resize(size_t(pos + data.length));

    bufferSlice = spBuffer->map();
    pInstance->Stream::setLength(bufferSlice.length);
  }

  memcpy((char*)bufferSlice.ptr + pos, data.ptr, data.length);
  pInstance->Stream::setPos(pos + data.length);

  pInstance->Broadcaster::write(data);

  return data.length;
}

int64_t MemStreamImpl::Seek(SeekOrigin rel, int64_t offset)
{
  int64_t pos = pInstance->Stream::getPos();
  int64_t length = pInstance->Stream::length();

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

  pInstance->Stream::setPos(pos);
  pInstance->Stream::posChanged.Signal();

  return pos;
}

} // namespace ep
