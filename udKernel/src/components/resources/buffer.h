#pragma once
#ifndef _UD_BUFFER_H
#define _UD_BUFFER_H

#include "components/resources/resource.h"
#include "util/udsharedptr.h"
#include "util/udstring.h"

namespace ud
{

PROTOTYPE_COMPONENT(Buffer);

class Buffer : public Resource
{
public:
  UD_COMPONENT(Buffer);

  void Allocate(size_t size);
  void Free();

  void Resize(size_t size);
  size_t GetBufferSize() const;

  void* Map(size_t *pSize = nullptr);
  const void* MapForRead(size_t *pSize = nullptr);
  void Unmap();

  void CopyBuffer(BufferRef buffer);
  void CopyBuffer(const void *pBuffer, size_t size);

protected:
  Buffer(const ComponentDesc *pType, Kernel *pKernel, udSharedString uid, udInitParams initParams)
    : Resource(pType, pKernel, uid, initParams) {}
  virtual ~Buffer() { Free(); }

  void _Resize(size_t size, bool copy);

  udSlice<char> buffer;

  size_t logicalSize = 0;
  int mapDepth = 0;
  bool readMap;
};

} // namespace ud

#endif // _UD_BUFFER_H
