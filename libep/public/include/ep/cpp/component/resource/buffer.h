#pragma once
#ifndef _EP_BUFFER_H
#define _EP_BUFFER_H

#include "ep/cpp/component/resource/resource.h"
#include "ep/cpp/internal/i/ibuffer.h"

namespace ep {

SHARED_CLASS(Buffer);

class Buffer : public Resource
{
  EP_DECLARE_COMPONENT_WITH_IMPL(ep, Buffer, IBuffer, Resource, EPKERNEL_PLUGINVERSION, "Binary buffer object", 0)
public:

  bool Reserve(size_t bytes) { return pImpl->Reserve(bytes); }
  virtual bool Allocate(size_t bytes) { return pImpl->Allocate(bytes); }
  bool Free() { return pImpl->Free(); }

  virtual bool Resize(size_t bytes) { return ResizeInternal(bytes, true); }

  bool Empty() const { return pImpl->Empty(); }
  size_t GetBufferSize() const { return pImpl->GetBufferSize(); }

  Slice<void> Map() { return pImpl->Map(); }
  Slice<const void> MapForRead() { return pImpl->MapForRead(); }
  void Unmap() { pImpl->Unmap(); }

  bool CopyBuffer(BufferRef buffer) { return pImpl->CopyBuffer(buffer); }
  bool CopyBuffer(Slice<const void> buffer) { return pImpl->CopyBuffer(buffer); }

  Variant Save() const override { return pImpl->Save(); }

protected:
  Buffer(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
    : Resource(pType, pKernel, uid, initParams)
  {
    pImpl = CreateImpl(initParams);
  }
  virtual ~Buffer() { Free(); }

  bool ResizeInternal(size_t bytes, bool copy) { return pImpl->ResizeInternal(bytes, copy); }

private:
  bool CopyBufferMethod(BufferRef _buffer) { return CopyBuffer(_buffer); };

  Array<const PropertyInfo> GetProperties() const;
  Array<const MethodInfo> GetMethods() const;
};

} // namespace ep

#endif // _EP_BUFFER_H
