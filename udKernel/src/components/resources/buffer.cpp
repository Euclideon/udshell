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

void Buffer::Allocate(size_t size)
{
  buffer.length = size;
  buffer.ptr = (char*)udAlloc(size);
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
  if (mapDepth > 0)
    return nullptr;
  readMap = false;
  ++mapDepth;
  if (pSize)
    *pSize = buffer.length;
  return buffer.ptr;
}

const void* Buffer::MapForRead(size_t *pSize)
{
  if (mapDepth > 0 && !readMap)
    return nullptr;
  readMap = true;
  ++mapDepth;
  if (pSize)
    *pSize = buffer.length;
  return buffer.ptr;
}

void Buffer::UnMap()
{
  if (--mapDepth == 0 && !readMap)
    changed.Signal();
}

void Buffer::SetBuffer(const void *pBuffer, size_t size)
{
  Free();
  Allocate(size);
  memcpy(buffer.ptr, pBuffer, size);
  changed.Signal();
}

} // namespace ud
