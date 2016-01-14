#pragma once
#ifndef _EP_BUFFER_H
#define _EP_BUFFER_H

#include "ep/cpp/component/resource/resource.h"
#include "ep/cpp/sharedptr.h"

namespace ep
{

SHARED_CLASS(Buffer);

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
  Buffer(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
    : Resource(pType, pKernel, uid, initParams) {}
  virtual ~Buffer() { Free(); }

  bool ResizeInternal(size_t size, bool copy);

  Slice<void> buffer;

  size_t logicalSize = 0;
  int mapDepth = 0;
  bool readMap;
};

} // namespace ep

#endif // _EP_BUFFER_H
