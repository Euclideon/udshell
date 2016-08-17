#pragma once
#ifndef _EP_ARRAYBUFFER_H
#define _EP_ARRAYBUFFER_H

#include "ep/cpp/component/resource/buffer.h"
#include "ep/cpp/internal/i/iarraybuffer.h"

#include "ep/cpp/stringof.h"
#include "ep/cpp/enum.h"
#include <utility>

namespace ep {

EP_BITFIELD(ElementInfoFlags,
  Signed,
  Float,
  Color
  );

struct ElementInfo
{
  size_t size;
  SharedArray<size_t> dimensions; // TODO : Support aggregates, union dimensions with a SharedArray<ElementInfo>.
  ElementInfoFlags flags;

  static ElementInfo parse(String str);
  SharedString asString() const;
  static SharedString buildTypeString(Slice<const ElementInfo> elements);
};

struct ElementMetadata
{
  SharedString name;
  SharedString type;
  ElementInfo info;
  size_t offset;
};

SHARED_CLASS(ArrayBuffer);

class ArrayBuffer : public Buffer
{
  EP_DECLARE_COMPONENT_WITH_IMPL(ep, ArrayBuffer, IArrayBuffer, Buffer, EPKERNEL_PLUGINVERSION, "Buffer object for arrays of elements", 0)
public:

  // array allocation
  void allocate(SharedString elementType, size_t elementSize, size_t length)
  {
    allocate(elementType, elementSize, Slice<size_t>(&length, 1));
  }

  void allocate(SharedString elementType, size_t elementSize, Slice<const size_t> shape)
  {
    pImpl->Allocate(elementType, elementSize, shape);
  }

  // strongly typed array allocation
  template<typename ElementType>
  void allocate(size_t length)
  {
    allocate<ElementType>(Slice<size_t>(&length, 1));
  }

  template<typename ElementType>
  void allocate(Slice<const size_t> shape)
  {
    allocate(stringof<ElementType>(), sizeof(ElementType), shape);
  }

  // allocate from existing data
  template<typename ElementType>
  void allocateFromData(Slice<const ElementType> data)
  {
    allocate<ElementType>(data.length);

    // initialise buffer
    Slice<ElementType> buffer = map<ElementType>();
    if (std::is_pod<ElementType>::value)
      memcpy(buffer.ptr, data.ptr, sizeof(ElementType)*data.length);
    else
    {
      for (size_t i = 0; i < data.length; ++i)
        epConstruct(buffer.ptr + i) ElementType(data.ptr[i]);
    }
    unmap();
  }

  bool reshape(size_t elements) { return reshape(Slice<size_t>(&elements, 1)); }
  bool reshape(Slice<const size_t> shape) { return reshapeInternal(shape, true); }

  SharedString getElementType() const { return pImpl->GetElementType(); }
  size_t getElementSize() const { return pImpl->GetElementSize(); }
  size_t getNumDimensions() const { return pImpl->GetNumDimensions(); }

  size_t getLength() const { return pImpl->GetLength(); }
  Slice<const size_t> getShape() const { return pImpl->GetShape(); }

  // ArrayBuffer overrides the map functions
  template<typename T = void>
  Slice<T> map()
  {
    EPASSERT_THROW(stringof<T>().eq(getElementType()), Result::InvalidType, "Incompatible type!");
    Slice<void> _buffer = Buffer::map();
    return Slice<T>((T*)_buffer.ptr, _buffer.length/sizeof(T));
  }

  template<typename T = void>
  Slice<const T> mapForRead()
  {
    EPASSERT_THROW(stringof<T>().eq(getElementType()), Result::InvalidType, "Incompatible type!");
    Slice<const void> buffer = Buffer::mapForRead();
    return Slice<const T>((const T*)buffer.ptr, buffer.length/sizeof(T));
  }

  template<typename T>
  void setData(Slice<const T> data)
  {
    EPASSERT_THROW(stringof<T>().eq(getElementType()), Result::InvalidType, "Incompatible type!");

    Slice<T> buffer = map<T>();
    epscope(exit) { unmap(); };

    EPASSERT_THROW(buffer.length == data.length, Result::InvalidArgument, "Incorrect number of elements!");

    // initialise buffer
    if (std::is_pod<T>::value)
      memcpy(buffer.ptr, data.ptr, sizeof(T)*data.length);
    else
    {
      for (size_t i = 0; i < data.length; ++i)
        epConstruct(buffer.ptr + i) T(data[i]);
    }
  }

  Variant save() const override { return pImpl->Save(); }

protected:
  friend class GeomNode;

  ArrayBuffer(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
    : Buffer(pType, pKernel, uid, initParams)
  {
    pImpl = createImpl(initParams);
  }

  bool reshapeInternal(Slice<const size_t> shape, bool copy) { return pImpl->ReshapeInternal(shape, copy); }

private:
  void allocateMethod(SharedString _elementType, size_t _elementSize, Variant length)
  {
    if (length.is(Variant::Type::Int))
    {
      size_t len = length.as<size_t>();
      allocate(_elementType, _elementSize, Slice<size_t>(&len, 1));
    }
    else
      allocate(_elementType, _elementSize, length.as<Array<size_t>>());
  }

  // Buffer's raw alloc/resize functions are hidden at this level of abstraction
  bool allocate(size_t) override final { EPASSERT_THROW(false, Result::InvalidCall, "Can't call Buffer::Allocate for ArrayBuffer!"); return false; }
  bool resize(size_t) override final { EPASSERT_THROW(false, Result::InvalidCall, "Can't call Buffer::Resize for ArrayBuffer!"); return false; }

  Array<const PropertyInfo> getProperties() const;
  Array<const MethodInfo> getMethods() const;
};

template<>
inline Slice<void> ArrayBuffer::map<void>() { return Buffer::map(); }

template<>
inline Slice<const void> ArrayBuffer::mapForRead<void>() { return Buffer::mapForRead(); }

inline Variant epToVariant(const ElementInfo &e)
{
  Variant::VarMap::MapType r;
  r.insert("flags", e.flags);
  r.insert("size", e.size);
  r.insert("dimensions", e.dimensions);
  return std::move(r);
}

inline void epFromVariant(const Variant &v, ElementInfo *pE)
{
  *pE = ElementInfo{ 0, { 0 }, 0 };

  Variant *pI = v.getItem("flags");
  if (pI)
    pE->flags = pI->as<ElementInfoFlags>();
  pI = v.getItem("size");
  if (pI)
    pE->size = pI->as<size_t>();
  pI = v.getItem("dimensions");
  if (pI)
    pE->dimensions = pI->as<SharedArray<size_t>>();
}

inline Variant epToVariant(const ElementMetadata &e)
{
  Variant::VarMap::MapType r;
  if (e.name)
    r.insert("name", e.name);
  if (e.type)
    r.insert("type", e.type);
  if (e.info.size)
    r.insert("info", epToVariant(e.info));
  if (e.type || e.offset)
    r.insert("offset", e.offset);
  return std::move(r);
}

inline void epFromVariant(const Variant &v, ElementMetadata *pE)
{
  *pE = ElementMetadata{ nullptr, nullptr, { 0, { 0 }, 0 }, 0 };

  Variant *pI = v.getItem("name");
  if (pI)
    pE->name = pI->asSharedString();
  pI = v.getItem("type");
  if (pI)
    pE->type = pI->asSharedString();
  pI = v.getItem("info");
  if (pI)
    epFromVariant(*pI, &pE->info);
  pI = v.getItem("offset");
  if (pI)
    pE->offset = pI->as<size_t>();
}

inline SharedString ElementInfo::asString() const
{
  EPASSERT_THROW(size > 0, Result::Failure, "Invalid size");

  MutableString<0> str;

  if (flags & ElementInfoFlags::Color)
    str.concat("color");
  else if (flags & ElementInfoFlags::Float)
    str.concat("f");
  else if (flags & ElementInfoFlags::Signed)
    str.concat("s");
  else
    str.concat("u");

  size_t div = 1;
  for (auto d : dimensions)
    div *= d;
  str.append(size / div * 8);

  if (dimensions.length && (dimensions[0] > 1 || dimensions.length > 1))
  {
    for (auto d : dimensions)
        str.append("[", d, "]");
  }

  return std::move(str);
}

inline ElementInfo ElementInfo::parse(String s)
{
  ElementInfo info = ElementInfo{ 0, { 0 }, 0 };
  EPASSERT_THROW(s, Result::InvalidArgument, "String is empty");

  s = s.trim();
  if (s.length > 2 && s.front() == '{' && s.back() == '}')
    s = s.slice(1, s.length - 1).trim();

  switch (s[0])
  {
    case 'c':
    {
      EPASSERT_THROW(s.slice(0, 5).eqIC("color"), Result::InvalidArgument, "The string does not represent a valid ElementInfo");
      info.flags |= ElementInfoFlags::Color;
      info.size = uint16_t(s.slice(5, s.length).parseInt() / 8);
      info.dimensions = SharedArray<size_t>{ 1 };
      break;
    }
    case 'f':
      info.flags |= ElementInfoFlags::Float;
    case 's':
      info.flags |= ElementInfoFlags::Signed;
    case 'u':
    {
      size_t array = s.findFirst('[');
      size_t md = 1;
      if (array != s.length)
      {
        String a = s.slice(array, s.length);
        Array<size_t> dims;
        a.tokenise([&dims, &md](String a, size_t i) mutable {
            a = a.trim();
            a.popBack();
            size_t d = size_t(a.parseInt());
            md *= d;
            dims.pushBack(d);
        }, "[");

        info.dimensions = dims;
      }

      info.size = (uint16_t)s.slice(1, array).parseInt() / 8 * md;
      break;
    }
    default:
      EPASSERT_THROW(false, Result::InvalidArgument, "The string does not represent a valid ElementInfo");
      break;
  }

  return info;
}

inline SharedString ElementInfo::buildTypeString(Slice<const ElementInfo> elements)
{
  bool first = true;
  MutableString<0> typeStr("{");
  for (auto &e : elements)
  {
    if (!first)
      typeStr.append(",");
    else
      first = false;
    typeStr.append(e.asString());
  }
  typeStr.append("}");
  return typeStr;
}

} // namespace ep

#endif // _EP_ARRAYBUFFER_H
