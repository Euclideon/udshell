#include "bufferimpl.h"

namespace ep {

Array<const PropertyInfo> Buffer::getProperties() const
{
  return{
    EP_MAKE_PROPERTY_RO("empty", empty, "Is the Buffer Empty", nullptr, 0),
    EP_MAKE_PROPERTY_RO("bufferSize", getBufferSize, "The buffer's size", nullptr, 0),
    EP_MAKE_PROPERTY_RO("mapped", mapped, "Is the Buffer Mapped", nullptr, 0),
  };
}
Array<const MethodInfo> Buffer::getMethods() const
{
  return{
    EP_MAKE_METHOD_EXPLICIT("copyBuffer", copyBufferMethod, "Copy the given Buffer to this Buffer"),
    EP_MAKE_METHOD(reserve, "Reserves the specified bytes of memory for the internal buffer"),
    EP_MAKE_METHOD(allocate, "Reserves the specified bytes of memory and sets BufferSize to the number of bytes allocated"),
    EP_MAKE_METHOD(resize, "Increase the logical size of the buffer, more memory will be allocated if necessary"),
    EP_MAKE_METHOD(free, "Free the internal buffer"),
  };
}

bool BufferImpl::Reserve(size_t size)
{
  EPTHROW_IF(mapDepth > 0, Result::ResourceInUse, "Buffer is mapped for {0}", readMap ? "reading" : "writing");

  if (size <= buffer.length)
    return true;

  Free();
  buffer.ptr = (char*)epAlloc(size);
  buffer.length = size;
  return true;
}

bool BufferImpl::Allocate(size_t size)
{
  if (!Reserve(size))
    return false;
  logicalSize = size;
  return true;
}

bool BufferImpl::Free()
{
  if (buffer.ptr && mapDepth == 0)
  {
    epFree(buffer.ptr);
    buffer.ptr = nullptr;
    buffer.length = 0;
  }
  else
    return false; // TODO Error handling

  return true;
}

bool BufferImpl::ResizeInternal(size_t size, bool copy)
{
  if (mapDepth > 0)
    return false; // TODO Error handling

  if (!buffer.ptr)
  {
    Allocate(size);
    return true; // TODO Error handling
  }

  size_t physSize = buffer.length;
  while (size > physSize)
    physSize *= 2;

  if (physSize > buffer.length)
  {
    char *newBuf = (char*)epAlloc(physSize);
    if (buffer.ptr)
    {
      if (copy)
        memcpy(newBuf, buffer.ptr, logicalSize);
      epFree(buffer.ptr);
    }
    buffer.ptr = newBuf;
    buffer.length = physSize;
  }

  logicalSize = size;
  return true; // TODO Error handling
}

size_t BufferImpl::GetBufferSize() const
{
  return logicalSize;
}

Slice<void> BufferImpl::Map()
{
  if (!buffer.ptr || mapDepth > 0)
    return nullptr;
  readMap = false;
  ++mapDepth;
  return Slice<void>(buffer.ptr, logicalSize);
}

Slice<const void> BufferImpl::MapForRead()
{
  if (!buffer.ptr || (mapDepth > 0 && !readMap))
    return nullptr;
  readMap = true;
  ++mapDepth;
  return Slice<void>(buffer.ptr, logicalSize);
}

void BufferImpl::Unmap()
{
  if (--mapDepth == 0 && !readMap)
    pInstance->changed.Signal();
}

bool BufferImpl::CopyBuffer(BufferRef buf)
{
  if (mapDepth > 0)
    return false; // TODO Error handling

  Slice<const void> mBuf = buf->mapForRead();
  EPASSERT(mBuf != nullptr, "Unable to map buffer!");
  if (mBuf != nullptr)
  {
    CopySlice(mBuf);
    buf->unmap();
  }

  return true; // TODO Error handling
}

bool BufferImpl::CopySlice(Slice<const void> buf)
{
  if (mapDepth > 0)
    return false; // TODO Error handling

  if (logicalSize < buf.length)
    ResizeInternal(buf.length, false);

  memcpy(buffer.ptr, buf.ptr, buf.length);

  pInstance->changed.Signal();
  return true; // TODO Error handling
}

} // namespace ep
