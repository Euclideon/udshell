#include "buffer.h"

namespace ud
{

ComponentDesc Buffer::descriptor =
{
  &Resource::descriptor, // pSuperDesc

  UDSHELL_APIVERSION, // udVersion
  UDSHELL_PLUGINVERSION, // pluginVersion

  "buffer", // id
  "Buffer", // displayName
  "Buffer resource", // description
};

void Buffer::Allocate(size_t size, AllocMode mode)
{
  Free();

  bufferLength = size;

  lockMode = mode;
  switch (mode)
  {
    case AllocMode::LockExisting:
      buffer = udRCSlice<char>::alloc(size);
      break;
  }
}

void Buffer::Free()
{
  buffer = nullptr;
}

size_t Buffer::GetBufferSize() const
{
  return bufferLength;
}

void* Buffer::Map(size_t *pSize)
{
  if (mapDepth > 0)
    return nullptr;

  readMap = false;
  ++mapDepth;

  switch (lockMode)
  {
    case AllocMode::LockExisting:
    {
      // TODO: verify that this is a reasonable thing to do
      // we can't lock a buffer if someone else has a ref to it... right>?
      if (buffer.refcount() > 1)
      {
        if (pSize)
          *pSize = 0;
        return nullptr;
      }
      break;
    }
    case AllocMode::CopyOnWrite:
    {
      udRCSlice<char> newBuffer = udRCSlice<char>::alloc(bufferLength);
      if (buffer.ptr)
        memcpy(newBuffer.ptr, buffer.ptr, bufferLength);
      buffer = newBuffer;
      break;
    }
    case AllocMode::DiscardOnWrite:
    {
      buffer = udRCSlice<char>::alloc(bufferLength);
      break;
    }
    default:
      UDUNREACHABLE();
      break;
  }

  if (pSize)
    *pSize = bufferLength;
  return buffer.ptr;
}

const void* Buffer::MapForRead(size_t *pSize)
{
  if (mapDepth > 0 && !readMap)
    return nullptr;
  readMap = true;
  ++mapDepth;
  if (pSize && buffer.ptr)
    *pSize = buffer.length;
  return buffer.ptr;
}

void Buffer::Unmap()
{
  if (--mapDepth == 0 && !readMap)
    changed.Signal();
}

void Buffer::CopyBuffer(BufferRef buffer)
{
  size_t size;
  const void *pBuffer = buffer->MapForRead(&size);
  UDASSERT(pBuffer != nullptr, "Unable to map buffer!");
  if (pBuffer)
  {
    CopyBuffer(pBuffer, size);
    buffer->Unmap();
  }
}

void Buffer::CopyBuffer(const void *pBuffer, size_t size)
{
  switch (lockMode)
  {
    case AllocMode::LockExisting:
      UDASSERT(buffer.ptr, "Buffer not allocated!");
      if (size != bufferLength)
        buffer = udRCSlice<char>::alloc(size);
      break;
    case AllocMode::CopyOnWrite:
    case AllocMode::DiscardOnWrite:
      buffer = udRCSlice<char>::alloc(size);
      break;
    default:
      UDUNREACHABLE();
      break;
  }

  bufferLength = size;
  memcpy(buffer.ptr, pBuffer, size);

  changed.Signal();
}

} // namespace ud
