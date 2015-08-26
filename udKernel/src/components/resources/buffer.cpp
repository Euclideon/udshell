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

void Buffer::Allocate(size_t size)
{
  Free();

  buffer.ptr = (char*)udAlloc(size);
  buffer.length = size;
}

void Buffer::Free()
{
  if (buffer.ptr)
  {
    udFree(buffer.ptr);
    buffer.ptr = nullptr;
    buffer.length = 0;
  }
}

size_t Buffer::GetBufferSize() const
{
  return buffer.length;
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
    *pSize = buffer.length;
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
    *pSize = buffer.length;
  return buffer.ptr;
}

void Buffer::Unmap()
{
  if (--mapDepth == 0 && !readMap)
    Changed.Signal();
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
  if (buffer.length < size)
  {
    if (buffer.ptr)
      udFree(buffer.ptr);
    buffer.ptr = (char*)udAlloc(size);
    buffer.length = size;
  }

  memcpy(buffer.ptr, pBuffer, size);

  Changed.Signal();
}

} // namespace ud
