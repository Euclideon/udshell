#pragma once
#ifndef _EP_ARRAYBUFFER_H
#define _EP_ARRAYBUFFER_H

#include "components/resources/buffer.h"
#include "ep/cpp/sharedptr.h"
#include "ep/epstringof.h"

#include <type_traits>

namespace ep
{

SHARED_CLASS(RenderResource);

PROTOTYPE_COMPONENT(ArrayBuffer);

class ArrayBuffer : public Buffer
{
public:
  EP_COMPONENT(ArrayBuffer);

  // array allocation
  inline void Allocate(SharedString elementType, size_t elementSize, size_t length)
  {
    Allocate(elementType, elementSize, Slice<size_t>(&length, 1));
  }
  void Allocate(SharedString elementType, size_t elementSize, Slice<const size_t> shape)
  {
    EPASSERT(shape.length > 0, "No dimensions given!");
    EPASSERT(shape.length <= 4, "More than 4 dimensional matrices is not supported...");

    dimensions = shape.length;
    this->elementType = elementType;

    // record dimensions and count total number of elements
    size_t elements = 1;
    for (size_t i = 0; i<shape.length; ++i)
    {
      this->shape[i] = shape.ptr[i];
      elements *= shape.ptr[i];
    }

    // alloc array
    bool alreadyAllocated = buffer.ptr != nullptr;
    this->elementSize = elementSize;
    Buffer::Allocate(elementSize*elements);
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
    Slice<void> buffer = Buffer::Map();
    return Slice<T>((T*)buffer.ptr, buffer.length/sizeof(T));
  }

  template<typename T = void>
  Slice<const T> MapForRead()
  {
    EPASSERT(stringof<T>().eq(elementType), "Incompatible type!");
    Slice<void> buffer = Buffer::MapForRead();
    if (pNumElements)
      *pNumElements = buffer.length/sizeof(T);
    return Slice<const T>((const T*)buffer.ptr, buffer.length/sizeof(T));
  }

  template<typename T>
  void SetData(Slice<const T> data)
  {
    EPASSERT(stringof<T>().eq(elementType), "Incompatible type!");

    size_t numElements = GetLength();
    EPASSERT(buffer.length == sizeof(T)*numElements, "Incorrect number of elements!");

    // initialise buffer
    if (std::is_pod<T>::value)
      memcpy(buffer.ptr, data.ptr, sizeof(T)*data.length);
    else
    {
      T *pBuffer = (T*)buffer.ptr;
      for (size_t i = 0; i < numElements; ++i)
        new(pBuffer + i) T(data[i]);
    }
  }

protected:
  friend class GeomNode;

  ArrayBuffer(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, InitParams initParams)
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
