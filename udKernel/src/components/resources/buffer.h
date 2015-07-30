#pragma once
#ifndef _UD_BUFFER_H
#define _UD_BUFFER_H

#include "components/resources/resource.h"
#include "util/udsharedptr.h"
#include "util/udstring.h"

namespace ud
{

SHARED_CLASS(Buffer);

class Buffer : public Resource
{
public:
  UD_COMPONENT(Buffer);

  void Allocate(size_t size);
  void Free();

  size_t GetBufferSize() const;

  void* Map(size_t *pSize = nullptr);
  const void* MapForRead(size_t *pSize = nullptr);
  void UnMap();

  void SetBuffer(const void *pBuffer, size_t size);

  udEvent<> changed;

protected:
  Buffer(const ComponentDesc *pType, Kernel *pKernel, udRCString uid, InitParams initParams)
    : Resource(pType, pKernel, uid, initParams) {}
  virtual ~Buffer() { Free(); }

  udSlice<char> buffer = nullptr;
  int mapDepth = 0;
  bool readMap;
};

} // namespace ud

#endif // _UD_BUFFER_H
