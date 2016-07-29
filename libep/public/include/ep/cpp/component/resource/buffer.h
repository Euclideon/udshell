#pragma once
#ifndef _EP_BUFFER_H
#define _EP_BUFFER_H

#include "ep/cpp/component/resource/resource.h"
#include "ep/cpp/internal/i/ibuffer.h"

#ifdef free
# undef free
#endif

namespace ep {

SHARED_CLASS(Buffer);

class Buffer : public Resource
{
  EP_DECLARE_COMPONENT_WITH_IMPL(ep, Buffer, IBuffer, Resource, EPKERNEL_PLUGINVERSION, "Binary buffer object", 0)
public:

  bool reserve(size_t bytes) { return pImpl->Reserve(bytes); }
  virtual bool allocate(size_t bytes) { return pImpl->Allocate(bytes); }
  bool free() { return pImpl->Free(); }

  virtual bool resize(size_t bytes) { return resizeInternal(bytes, true); }

  bool empty() const { return pImpl->Empty(); }
  size_t getBufferSize() const { return pImpl->GetBufferSize(); }

  Slice<void> map() { return pImpl->Map(); }
  Slice<const void> mapForRead() { return pImpl->MapForRead(); }
  void unmap() { pImpl->Unmap(); }

  bool mapped() const { return pImpl->Mapped(); }

  bool copyBuffer(BufferRef buffer) { return pImpl->CopyBuffer(buffer); }
  bool copyBuffer(Slice<const void> buffer) { return pImpl->CopySlice(buffer); }

  Variant save() const override { return pImpl->Save(); }

protected:
  Buffer(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
    : Resource(pType, pKernel, uid, initParams)
  {
    pImpl = createImpl(initParams);
  }
  virtual ~Buffer() { free(); }

  bool resizeInternal(size_t bytes, bool copy) { return pImpl->ResizeInternal(bytes, copy); }

private:
  bool copyBufferMethod(BufferRef _buffer) { return copyBuffer(_buffer); };

  Array<const PropertyInfo> getProperties() const;
  Array<const MethodInfo> getMethods() const;
};

} // namespace ep

#endif // _EP_BUFFER_H
