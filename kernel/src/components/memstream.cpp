#include "components/memstream.h"
#include "kernel.h"

namespace ep
{
static CPropertyDesc props[] =
{
  {
    {
      "buffer", // id
      "Buffer", // displayName
      "The Buffer component MemStream reads/writes to", // description
    },
    &MemStream::GetBuffer, // getter
    &MemStream::SetBuffer, // setter
  }
};

ComponentDesc MemStream::descriptor =
{
  &Stream::descriptor, // pSuperDesc

  EPSHELL_APIVERSION, // epVersion
  EPSHELL_PLUGINVERSION, // pluginVersion

  "memstream", // id
  "MemStream", // displayName
  "Memory stream", // description

  epSlice<CPropertyDesc>(props, UDARRAYSIZE(props)), // properties
  nullptr,
  nullptr
};

MemStream::MemStream(const ComponentDesc *pType, Kernel *pKernel, epSharedString uid, epInitParams initParams)
  : Stream(pType, pKernel, uid, initParams)
{
  BufferRef inBuffer;

  const epVariant &fl = initParams["flags"];
  oFlags = fl.as<OpenFlags>();
  if (!(oFlags & (OpenFlags::Write | OpenFlags::Read)))
    throw udR_InvalidParameter_;

  const epVariant &buf = initParams["buffer"];
  if (buf.is(epVariant::Type::Null))
  {
      inBuffer = pKernel->CreateComponent<Buffer>();
      inBuffer->Allocate(DefaultBufferSize);
  }
  else
  {
    inBuffer = buf.as<BufferRef>();
    if (!inBuffer)
      throw udR_InvalidParameter_;
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
    length = pos = 0;
  }

  spBuffer = spNewBuffer;

  if (!spBuffer)
    return;

  size_t len = 0;
  if (oFlags & OpenFlags::Write)
  {
    buffer = spBuffer->Map();
    if (!buffer)
      LogError("Can't reserve Buffer for writing.");
  }
  else if (oFlags & OpenFlags::Read)
  {
    auto map = spBuffer->MapForRead();
    buffer = epSlice<void>(const_cast<void*>(map.ptr), map.length);
    if (!buffer)
      LogError("Can't reserve Buffer for reading.");
  }
  length = len;

  if (!buffer)
  {
    spBuffer = nullptr;
    return; // TODO Error handling
  }
}

epSlice<void> MemStream::Read(epSlice<void> buffer)
{
  if (!spBuffer)
    return 0;

  size_t bytes = buffer.length;
  if (pos + bytes > (size_t)length)
    bytes = size_t(length - pos);

  memcpy(buffer.ptr, (const char*)buffer.ptr + pos, bytes);
  pos += bytes;

  return epSlice<void>(buffer.ptr, bytes);
}

size_t MemStream::Write(epSlice<const void> data)
{
  if (!(oFlags & OpenFlags::Write) || !spBuffer)
    return 0;

  if (pos + data.length > (size_t)length)
  {
    spBuffer->Unmap();
    spBuffer->Resize(size_t(pos + data.length));

    buffer = spBuffer->Map();
    length = buffer.length;
  }

  memcpy((char*)buffer.ptr + pos, data.ptr, data.length);
  pos += data.length;

  spBuffer->Changed.Signal();

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

  return pos;
}

} // namespace ep
