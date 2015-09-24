#include "buffer.h"
#include "renderresource.h"
#include "renderscene.h"

namespace ep
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

bool Buffer::Allocate(size_t size)
{
  if (mapDepth > 0)
    return false; // TODO Error handling

  Free();

  buffer.ptr = (char*)udAlloc(size);
  buffer.length = logicalSize = size;

  return true;
}

bool Buffer::Free()
{
  if (buffer.ptr && mapDepth == 0)
  {
    udFree(buffer.ptr);
    buffer.ptr = nullptr;
    buffer.length = 0;
  }
  else
    return false; // TODO Error handling

  return true;
}

bool Buffer::_Resize(size_t size, bool copy)
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
    char *newBuf = (char*)udAlloc(physSize);
    if (buffer.ptr)
    {
      if (copy)
        memcpy(newBuf, buffer.ptr, logicalSize);
      udFree(buffer.ptr);
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

epSlice<void> Buffer::Map()
{
  if (!buffer.ptr || mapDepth > 0)
    return nullptr;
  readMap = false;
  ++mapDepth;
  return epSlice<void>(buffer.ptr, logicalSize);
}

epSlice<const void> Buffer::MapForRead()
{
  if (!buffer.ptr || (mapDepth > 0 && !readMap))
    return nullptr;
  readMap = true;
  ++mapDepth;
  return epSlice<void>(buffer.ptr, logicalSize);
}

void Buffer::Unmap()
{
  if (--mapDepth == 0 && !readMap)
    Changed.Signal();
}

bool Buffer::CopyBuffer(BufferRef buffer)
{
  if (mapDepth > 0)
    return false; // TODO Error handling

  epSlice<const void> buf = buffer->MapForRead();
  EPASSERT(buf != nullptr, "Unable to map buffer!");
  if (buf != nullptr)
  {
    CopyBuffer(buf);
    buffer->Unmap();
  }

  return true; // TODO Error handling
}

bool Buffer::CopyBuffer(epSlice<const void> _buffer)
{
  if (mapDepth > 0)
    return false; // TODO Error handling

  if (logicalSize < _buffer.length)
    _Resize(_buffer.length, false);

  memcpy(buffer.ptr, _buffer.ptr, _buffer.length);

  Changed.Signal();
  return true; // TODO Error handling
}

} // namespace ep
