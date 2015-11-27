#pragma once
#ifndef _EP_BUFFER_H
#define _EP_BUFFER_H

#include "components/resources/resource.h"
#include "ep/cpp/sharedptr.h"

namespace kernel
{

PROTOTYPE_COMPONENT(Buffer);

class Buffer : public Resource
{
  EP_DECLARE_COMPONENT(Buffer, Resource, EPKERNEL_PLUGINVERSION, "Buffer desc...")
public:

  bool Reserve(size_t size);
  bool Allocate(size_t size);
  bool Free();

  bool Resize(size_t size) { return ResizeInternal(size, true); }
  size_t GetBufferSize() const;

  Slice<void> Map();
  Slice<const void> MapForRead();
  void Unmap();

  bool CopyBuffer(BufferRef buffer);
  bool CopyBuffer(Slice<const void> buffer);

protected:
  Buffer(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, InitParams initParams)
    : Resource(pType, pKernel, uid, initParams) {}
  virtual ~Buffer() { Free(); }

  bool ResizeInternal(size_t size, bool copy);

  Slice<void> buffer;

  size_t logicalSize = 0;
  int mapDepth = 0;
  bool readMap;
};

} // namespace kernel

#endif // _EP_BUFFER_H
