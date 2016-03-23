#pragma once
#ifndef _EP_BUFFER_H
#define _EP_BUFFER_H

#include "ep/cpp/component/resource/resource.h"
#include "ep/cpp/internal/i/ibuffer.h"

namespace ep {

SHARED_CLASS(Buffer);

class Buffer : public Resource
{
  EP_DECLARE_COMPONENT_WITH_IMPL(Buffer, IBuffer, Resource, EPKERNEL_PLUGINVERSION, "Buffer desc...", 0)
public:

  bool Reserve(size_t size) { return pImpl->Reserve(size); }
  bool Allocate(size_t size) { return pImpl->Allocate(size); }
  bool Free() { return pImpl->Free(); }

  bool Resize(size_t size) { return ResizeInternal(size, true); }

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

  bool ResizeInternal(size_t size, bool copy) { return pImpl->ResizeInternal(size, copy); }

  static Array<const PropertyInfo> GetProperties()
  {
    return{
      EP_MAKE_PROPERTY_RO(BufferSize, "The buffer's size", nullptr, 0),
    };
  }
  static Array<const MethodInfo> GetMethods()
  {
    return{
      EP_MAKE_METHOD_EXPLICIT("CopyBuffer", CopyBufferMethod, "Copy the given Buffer to this Buffer"),
      EP_MAKE_METHOD(Reserve, "Reserves the specified bytes of memory for the internal buffer"),
      EP_MAKE_METHOD(Allocate, "Reserves the specified bytes of memory and sets BufferSize to the number of bytes allocated"),
      EP_MAKE_METHOD(Resize, "Increase the logical size of the buffer, more memory will be allocated if necessary"),
      EP_MAKE_METHOD(Free, "Free the internal buffer"),
    };
  }

private:
  bool CopyBufferMethod(BufferRef _buffer) { return CopyBuffer(_buffer); };
};

} // namespace ep

#endif // _EP_BUFFER_H
