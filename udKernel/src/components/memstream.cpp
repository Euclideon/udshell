#include "components/memstream.h"
#include "kernel.h"

namespace ud
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

  UDSHELL_APIVERSION, // udVersion
  UDSHELL_PLUGINVERSION, // pluginVersion

  "memstream", // id
  "MemStream", // displayName
  "Memory stream", // description

  udSlice<CPropertyDesc>(props, UDARRAYSIZE(props)), // properties
  nullptr,
  nullptr
};

MemStream::MemStream(const ComponentDesc *pType, Kernel *pKernel, udSharedString uid, udInitParams initParams)
  : Stream(pType, pKernel, uid, initParams)
{
  BufferRef inBuffer;

  const udVariant &fl = initParams["flags"];
  oFlags = fl.as<OpenFlags>();
  if (!(oFlags & (OpenFlags::Write | OpenFlags::Read)))
    throw udR_InvalidParameter_;

  const udVariant &buf = initParams["buffer"];
  if (buf.is(udVariant::Type::Null))
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
    pBufArray = spBuffer->Map(&len);
    if (!pBufArray)
      LogError("Can't reserve Buffer for writing.");
  }
  else if (oFlags & OpenFlags::Read)
  {
    pBufArray = const_cast<void *>(spBuffer->MapForRead(&len));
    if (!pBufArray)
      LogError("Can't reserve Buffer for reading.");
  }
  length = len;

  if (!pBufArray)
  {
    spBuffer = nullptr;
    return; // TODO Error handling
  }
}

size_t MemStream::Read(void *pData, size_t bytes)
{
  if (!spBuffer)
    return 0;

  if (pos + bytes > (size_t)length)
    bytes = size_t(length - pos);

  memcpy(pData, (char *)pBufArray + pos, bytes);
  pos += bytes;

  return bytes;
}

size_t MemStream::Write(const void *pData, size_t bytes)
{
  if (!(oFlags & OpenFlags::Write) || !spBuffer)
    return 0;

  if (pos + bytes > (size_t)length)
  {
    spBuffer->Unmap();
    spBuffer->Resize(size_t(pos + bytes));

    size_t len = 0;
    pBufArray = spBuffer->Map(&len);
    length = len;
  }

  memcpy((char *)pBufArray + pos, pData, bytes);
  pos += bytes;

  spBuffer->Changed.Signal();

  return bytes;
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

} // namespace ud
