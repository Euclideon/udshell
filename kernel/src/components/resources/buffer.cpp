#include "buffer.h"
#include "renderresource.h"
#include "renderscene.h"

namespace ep
{

ComponentDesc Buffer::descriptor =
{
  &Resource::descriptor, // pSuperDesc

  EPSHELL_APIVERSION, // epVersion
  EPSHELL_PLUGINVERSION, // pluginVersion

  "buffer", // id
  "Buffer", // displayName
  "Buffer resource", // description
};

bool Buffer::Allocate(size_t size)
{
  if (mapDepth > 0)
    return false; // TODO Error handling

  Free();

  buffer.ptr = (char*)epAlloc(size);
  buffer.length = size;
  logicalSize = 0;

  return true;
}

bool Buffer::Free()
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

bool Buffer::ResizeInternal(size_t size, bool copy)
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

size_t Buffer::GetBufferSize() const
{
  return logicalSize;
}

Slice<void> Buffer::Map()
{
  if (!buffer.ptr || mapDepth > 0)
    return nullptr;
  readMap = false;
  ++mapDepth;
  return Slice<void>(buffer.ptr, logicalSize);
}

Slice<const void> Buffer::MapForRead()
{
  if (!buffer.ptr || (mapDepth > 0 && !readMap))
    return nullptr;
  readMap = true;
  ++mapDepth;
  return Slice<void>(buffer.ptr, logicalSize);
}

void Buffer::Unmap()
{
  if (--mapDepth == 0 && !readMap)
    Changed.Signal();
}

bool Buffer::CopyBuffer(BufferRef buf)
{
  if (mapDepth > 0)
    return false; // TODO Error handling

  Slice<const void> mBuf = buf->MapForRead();
  EPASSERT(mBuf != nullptr, "Unable to map buffer!");
  if (mBuf != nullptr)
  {
    CopyBuffer(mBuf);
    buf->Unmap();
  }

  return true; // TODO Error handling
}

bool Buffer::CopyBuffer(Slice<const void> buf)
{
  if (mapDepth > 0)
    return false; // TODO Error handling

  if (logicalSize < buf.length)
    ResizeInternal(buf.length, false);

  memcpy(buffer.ptr, buf.ptr, buf.length);

  Changed.Signal();
  return true; // TODO Error handling
}

} // namespace ep
