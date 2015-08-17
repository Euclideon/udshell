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

  UD_ENUM(AllocMode,
            LockExisting,
            CopyOnWrite,
            DiscardOnWrite
          );

  void Allocate(size_t size, AllocMode mode = AllocMode::LockExisting);
  void Free();

  size_t GetBufferSize() const;

  void* Map(size_t *pSize = nullptr);
  const void* MapForRead(size_t *pSize = nullptr);
  void Unmap();

  void CopyBuffer(BufferRef buffer);
  void CopyBuffer(const void *pBuffer, size_t size);

  udRCSlice<char> GetRawBuffer() { return buffer; }

  udEvent<> changed;

protected:
  Buffer(const ComponentDesc *pType, Kernel *pKernel, udRCString uid, udInitParams initParams)
    : Resource(pType, pKernel, uid, initParams) {}
  virtual ~Buffer() { Free(); }

  AllocMode lockMode = AllocMode::LockExisting;
  size_t bufferLength = 0;

  udRCSlice<char> buffer = nullptr;

  int mapDepth = 0;
  bool readMap;
};

} // namespace ud

#endif // _UD_BUFFER_H
