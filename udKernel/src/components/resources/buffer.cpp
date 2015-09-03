#include "buffer.h"
#include "renderresource.h"
#include "renderscene.h"

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

void* Buffer::Map(size_t *pSize)
{
  if (!buffer.ptr || mapDepth > 0)
  {
    if (pSize)
      *pSize = 0;
    return nullptr;
  }
  readMap = false;
  ++mapDepth;
  if (pSize)
    *pSize = logicalSize;
  return buffer.ptr;
}

const void* Buffer::MapForRead(size_t *pSize)
{
  if (!buffer.ptr || (mapDepth > 0 && !readMap))
  {
    if (pSize)
      *pSize = 0;
    return nullptr;
  }
  readMap = true;
  ++mapDepth;
  if (pSize)
    *pSize = logicalSize;
  return buffer.ptr;
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

  size_t size;
  const void *pBuffer = buffer->MapForRead(&size);
  UDASSERT(pBuffer != nullptr, "Unable to map buffer!");
  if (pBuffer)
  {
    CopyBuffer(pBuffer, size);
    buffer->Unmap();
  }

  return true; // TODO Error handling
}

bool Buffer::CopyBuffer(const void *pBuffer, size_t size)
{
  if (mapDepth > 0)
    return false; // TODO Error handling

  if (logicalSize < size)
    _Resize(size, false);

  memcpy(buffer.ptr, pBuffer, size);

  Changed.Signal();
  return true; // TODO Error handling
}

} // namespace ud
