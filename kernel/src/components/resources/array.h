#pragma once
#ifndef _EP_ARRAYBUFFER_H
#define _EP_ARRAYBUFFER_H

#include "components/resources/buffer.h"
#include "ep/cpp/sharedptr.h"
#include "ep/epstringof.h"

#include <utility>

namespace ep
{

SHARED_CLASS(RenderResource);

SHARED_CLASS(ArrayBuffer);

class ArrayBuffer : public Buffer
{
  EP_DECLARE_COMPONENT(ArrayBuffer, Buffer, EPKERNEL_PLUGINVERSION, "ArrayBuffer desc...")
public:

  // array allocation
  inline void Allocate(SharedString _elementType, size_t _elementSize, size_t length)
  {
    Allocate(_elementType, _elementSize, Slice<size_t>(&length, 1));
  }
  void Allocate(SharedString _elementType, size_t _elementSize, Slice<const size_t> _shape)
  {
    EPASSERT_THROW(_shape.length > 0, epR_InvalidArgument, "No dimensions given!");
    EPASSERT_THROW(_shape.length <= 4, epR_InvalidArgument, "More than 4 dimensional matrices is not supported...");

    dimensions = _shape.length;
    elementType = _elementType;

    // record dimensions and count total number of elements
    size_t elements = 1;
    for (size_t i = 0; i<_shape.length; ++i)
    {
      shape[i] = _shape.ptr[i];
      elements *= _shape.ptr[i];
    }

    // alloc array
    bool alreadyAllocated = buffer.ptr != nullptr;
    elementSize = _elementSize;
    Buffer::Allocate(_elementSize*elements);
    if (alreadyAllocated)
      Changed.Signal();
  }

  // strongly typed array allocation
  template<typename ElementType>
  inline void Allocate(size_t length)
  {
    Allocate<ElementType>(Slice<size_t>(&length, 1));
  }
  template<typename ElementType>
  inline void Allocate(Slice<const size_t> shape)
  {
    Allocate(stringof<ElementType>(), sizeof(ElementType), shape);
  }

  // allocate from existing data
  template<typename ElementType>
  inline void AllocateFromData(Slice<const ElementType> data)
  {
    Allocate<ElementType>(data.length);

    // initialise buffer
    if (std::is_pod<ElementType>::value)
      memcpy(buffer.ptr, data.ptr, sizeof(ElementType)*data.length);
    else
    {
      ElementType *pBuffer = (ElementType*)buffer.ptr;
      for (size_t i = 0; i < data.length; ++i)
        new(pBuffer + i) ElementType(data.ptr[i]);
    }
  }

  SharedString GetElementType() const { return elementType; }
  size_t GetElementSize() const { return elementSize; }
  size_t GetNumDimensions() const { return dimensions; }

  size_t GetLength() const
  {
    size_t elements = shape[0];
    for (size_t i = 1; i<dimensions; ++i)
      elements *= shape[i];
    return elements;
  }
  Slice<const size_t> GetShape() const
  {
    return Slice<const size_t>(shape, dimensions);
  }

  // ArrayBuffer overrides the map functions
  template<typename T = void>
  Slice<T> Map()
  {
    EPASSERT(stringof<T>().eq(elementType), "Incompatible type!");
    Slice<void> _buffer = Buffer::Map();
    return Slice<T>((T*)_buffer.ptr, _buffer.length/sizeof(T));
  }

  template<typename T = void>
  Slice<const T> MapForRead()
  {
    EPASSERT(stringof<T>().eq(elementType), "Incompatible type!");
    Slice<void> buffer = Buffer::MapForRead();
    return Slice<const T>((const T*)buffer.ptr, buffer.length/sizeof(T));
  }

  template<typename T>
  void SetData(Slice<const T> data)
  {
    EPASSERT_THROW(stringof<T>().eq(elementType), epR_InvalidType, "Incompatible type!");
    EPASSERT_THROW(buffer.length == sizeof(T)*data.length, epR_InvalidArgument, "Incorrect number of elements!");

    // initialise buffer
    if (std::is_pod<T>::value)
      memcpy(buffer.ptr, data.ptr, sizeof(T)*data.length);
    else
    {
      T *pBuffer = (T*)buffer.ptr;
      for (size_t i = 0; i < data.length; ++i)
        new(pBuffer + i) T(data[i]);
    }
  }

protected:
  friend class GeomNode;

  ArrayBuffer(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
    : Buffer(pType, pKernel, uid, initParams)
  {
    Changed.Subscribe(Delegate<void()>(this, &ArrayBuffer::OnBufferDirty));
  }
  ~ArrayBuffer()
  {
    Changed.Unsubscribe(Delegate<void()>(this, &ArrayBuffer::OnBufferDirty));
  }

  void OnBufferDirty()
  {
    spRenderData = nullptr;
  }

  enum class RenderResourceType
  {
    VertexArray,
    IndexArray,
    Texture,
  };
  RenderResourceRef GetRenderResource(RenderResourceType type);

  SharedString elementType;
  size_t elementSize;
  size_t dimensions;
  size_t shape[4];

  RenderResourceRef spRenderData = nullptr;
};

template<>
inline Slice<void> ArrayBuffer::Map<void>() { return Buffer::Map(); }

template<>
inline Slice<const void> ArrayBuffer::MapForRead<void>() { return Buffer::MapForRead(); }


} // namespace ep

#endif // _EP_ARRAYBUFFER_H
