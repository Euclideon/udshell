#pragma once
#ifndef _UD_ARRAYBUFFER_H
#define _UD_ARRAYBUFFER_H

#include "components/resources/buffer.h"
#include "util/udsharedptr.h"
#include "util/udstring.h"
#include "util/stringof.h"

#include <type_traits>

namespace ud
{

SHARED_CLASS(RenderResource);

PROTOTYPE_COMPONENT(ArrayBuffer);

class ArrayBuffer : public Buffer
{
public:
  UD_COMPONENT(ArrayBuffer);

  // array allocation
  inline void Allocate(udSharedString elementType, size_t elementSize, size_t length)
  {
    Allocate(elementType, elementSize, udSlice<size_t>(&length, 1));
  }
  void Allocate(udSharedString elementType, size_t elementSize, udSlice<const size_t> shape)
  {
    UDASSERT(shape.length > 0, "No dimensions given!");
    UDASSERT(shape.length <= 4, "More than 4 dimensional matrices is not supported...");

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
    Allocate<ElementType>(udSlice<size_t>(&length, 1));
  }
  template<typename ElementType>
  inline void Allocate(udSlice<const size_t> shape)
  {
    Allocate(stringof<ElementType>(), sizeof(ElementType), shape);
  }

  // allocate from existing data
  template<typename ElementType>
  inline void AllocateFromData(udSlice<const ElementType> data)
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

  udSharedString GetElementType() const { return elementType; }
  size_t GetElementSize() const { return elementSize; }
  size_t GetNumDimensions() const { return dimensions; }

  size_t GetLength() const
  {
    size_t elements = shape[0];
    for (size_t i = 1; i<dimensions; ++i)
      elements *= shape[i];
    return elements;
  }
  udSlice<const size_t> GetShape() const
  {
    return udSlice<const size_t>(shape, dimensions);
  }

  // ArrayBuffer overrides the map functions
  template<typename T = void>
  udSlice<T> Map()
  {
    UDASSERT(stringof<T>().eq(elementType), "Incompatible type!");
    udSlice<void> buffer = Buffer::Map();
    return udSlice<T>((T*)buffer.ptr, buffer.length/sizeof(T));
  }

  template<typename T = void>
  udSlice<const T> MapForRead()
  {
    UDASSERT(stringof<T>().eq(elementType), "Incompatible type!");
    udSlice<void> buffer = Buffer::MapForRead();
    if (pNumElements)
      *pNumElements = buffer.length/sizeof(T);
    return udSlice<const T>((const T*)buffer.ptr, buffer.length/sizeof(T));
  }

  template<typename T>
  void SetData(udSlice<const T> data)
  {
    UDASSERT(stringof<T>().eq(elementType), "Incompatible type!");

    size_t numElements = GetLength();
    UDASSERT(buffer.length == sizeof(T)*numElements, "Incorrect number of elements!");

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

  ArrayBuffer(const ComponentDesc *pType, Kernel *pKernel, udSharedString uid, udInitParams initParams)
    : Buffer(pType, pKernel, uid, initParams)
  {
    Changed.Subscribe(udDelegate<void()>(this, &ArrayBuffer::OnBufferDirty));
  }
  ~ArrayBuffer()
  {
    Changed.Unsubscribe(udDelegate<void()>(this, &ArrayBuffer::OnBufferDirty));
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

  udSharedString elementType;
  size_t elementSize;
  size_t dimensions;
  size_t shape[4];

  RenderResourceRef spRenderData = nullptr;
};

template<>
inline udSlice<void> ArrayBuffer::Map<void>() { return Buffer::Map(); }

template<>
inline udSlice<const void> ArrayBuffer::MapForRead<void>() { return Buffer::MapForRead(); }


} // namespace ud

#endif // _UD_ARRAYBUFFER_H
