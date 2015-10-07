#pragma once
#ifndef _EP_BUFFER_H
#define _EP_BUFFER_H

#include "components/resources/resource.h"
#include "ep/epsharedptr.h"

namespace ep
{

PROTOTYPE_COMPONENT(Buffer);

class Buffer : public Resource
{
public:
  EP_COMPONENT(Buffer);

  bool Allocate(size_t size);
  bool Free();

  bool Resize(size_t size) { return ResizeInternal(size, true); }
  size_t GetBufferSize() const;

  epSlice<void> Map();
  epSlice<const void> MapForRead();
  void Unmap();

  bool CopyBuffer(BufferRef buffer);
  bool CopyBuffer(epSlice<const void> buffer);

protected:
  Buffer(const ComponentDesc *pType, Kernel *pKernel, epSharedString uid, epInitParams initParams)
    : Resource(pType, pKernel, uid, initParams) {}
  virtual ~Buffer() { Free(); }

  bool ResizeInternal(size_t size, bool copy);

  epSlice<void> buffer;

  size_t logicalSize = 0;
  int mapDepth = 0;
  bool readMap;
};

} // namespace ep

#endif // _EP_BUFFER_H
