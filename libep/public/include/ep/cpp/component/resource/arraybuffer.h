#pragma once
#ifndef _EP_ARRAYBUFFER_H
#define _EP_ARRAYBUFFER_H

#include "ep/cpp/component/resource/buffer.h"
#include "ep/cpp/internal/i/iarraybuffer.h"

#include "ep/cpp/stringof.h"

#include <utility>

namespace ep {

SHARED_CLASS(ArrayBuffer);

class ArrayBuffer : public Buffer, public IArrayBuffer
{
  EP_DECLARE_COMPONENT_WITH_IMPL(ArrayBuffer, IArrayBuffer, Buffer, EPKERNEL_PLUGINVERSION, "ArrayBuffer desc...")
public:

  // array allocation
  inline void Allocate(SharedString _elementType, size_t _elementSize, size_t length)
  {
    Allocate(_elementType, _elementSize, Slice<size_t>(&length, 1));
  }
  void Allocate(SharedString _elementType, size_t _elementSize, Slice<const size_t> _shape) override final
  {
    pImpl->Allocate(_elementType, _elementSize, _shape);
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
    Slice<ElementType> buffer = Map<ElementType>();
    if (std::is_pod<ElementType>::value)
      memcpy(buffer.ptr, data.ptr, sizeof(ElementType)*data.length);
    else
    {
      for (size_t i = 0; i < data.length; ++i)
        new(buffer.ptr + i) ElementType(data.ptr[i]);
    }
    Unmap();
  }

  SharedString GetElementType() const override final { return pImpl->GetElementType(); }
  size_t GetElementSize() const override final { return pImpl->GetElementSize(); }
  size_t GetNumDimensions() const override final { return pImpl->GetNumDimensions(); }

  size_t GetLength() const override final { return pImpl->GetLength(); }
  Slice<const size_t> GetShape() const override final { return pImpl->GetShape(); }

  // ArrayBuffer overrides the map functions
  template<typename T = void>
  Slice<T> Map()
  {
    EPASSERT(stringof<T>().eq(GetElementType()), "Incompatible type!");
    Slice<void> _buffer = Buffer::Map();
    return Slice<T>((T*)_buffer.ptr, _buffer.length/sizeof(T));
  }

  template<typename T = void>
  Slice<const T> MapForRead()
  {
    EPASSERT(stringof<T>().eq(GetElementType()), "Incompatible type!");
    Slice<const void> buffer = Buffer::MapForRead();
    return Slice<const T>((const T*)buffer.ptr, buffer.length/sizeof(T));
  }

  template<typename T>
  void SetData(Slice<const T> data)
  {
    EPASSERT_THROW(stringof<T>().eq(GetElementType()), epR_InvalidType, "Incompatible type!");

    Slice<T> buffer = Map<T>();
    epscope(exit) { Unmap(); };

    EPASSERT_THROW(buffer.length == data.length, epR_InvalidArgument, "Incorrect number of elements!");

    // initialise buffer
    if (std::is_pod<T>::value)
      memcpy(buffer.ptr, data.ptr, sizeof(T)*data.length);
    else
    {
      for (size_t i = 0; i < data.length; ++i)
        new(buffer.ptr + i) T(data[i]);
    }
  }

  Variant Save() const override { return pImpl->Save(); }

protected:
  friend class GeomNode;

  ArrayBuffer(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
    : Buffer(pType, pKernel, uid, initParams)
  {
    pImpl = CreateImpl(initParams);
  }

  Array<const PropertyInfo> GetProperties()
  {
    return{
      EP_MAKE_PROPERTY_RO(ElementType, "Element type of the Array as a String", nullptr, 0),
      EP_MAKE_PROPERTY_RO(ElementSize, "The Array's element size", nullptr, 0),
      EP_MAKE_PROPERTY_RO(NumDimensions, "Number of dimensions", nullptr, 0),
      EP_MAKE_PROPERTY_RO(Length, "Number of Array elements", nullptr, 0),
      EP_MAKE_PROPERTY_RO(Shape, "The sizes of the Array's dimensions", nullptr, 0),
    };
  }

  static Array<const MethodInfo> GetMethods()
  {
    return{
      EP_MAKE_METHOD_EXPLICIT("Allocate", AllocateMethod, "Allocates an Array of the given type, size and length. Length may be a single value or a list of dimensions"),
    };
  }

private:
  void AllocateMethod(SharedString _elementType, size_t _elementSize, Variant length)
  {
    if (length.is(Variant::Type::Int))
    {
      size_t len = length.as<size_t>();
      Allocate(_elementType, _elementSize, Slice<size_t>(&len, 1));
    }
    else
      Allocate(_elementType, _elementSize, length.as<Array<size_t>>());
  }
};

template<>
inline Slice<void> ArrayBuffer::Map<void>() { return Buffer::Map(); }

template<>
inline Slice<const void> ArrayBuffer::MapForRead<void>() { return Buffer::MapForRead(); }

} // namespace ep

#endif // _EP_ARRAYBUFFER_H
