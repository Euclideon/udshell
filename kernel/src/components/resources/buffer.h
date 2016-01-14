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
