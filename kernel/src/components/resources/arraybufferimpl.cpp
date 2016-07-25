#include "arraybufferimpl.h"
#include "renderresource.h"
#include "ep/cpp/kernel.h"
#include "ep/cpp/component/resource/metadata.h"

namespace ep {

Array<const PropertyInfo> ArrayBuffer::GetProperties() const
{
  return{
    EP_MAKE_PROPERTY_RO("elementType", GetElementType, "Element type of the Array as a String", nullptr, 0),
    EP_MAKE_PROPERTY_RO("elementSize", GetElementSize, "The Array's element size", nullptr, 0),
    EP_MAKE_PROPERTY_RO("numDimensions", GetNumDimensions, "Number of dimensions", nullptr, 0),
    EP_MAKE_PROPERTY_RO("length", GetLength, "Number of Array elements", nullptr, 0),
    EP_MAKE_PROPERTY_RO("shape", GetShape, "The sizes of the Array's dimensions", nullptr, 0),
  };
}
Array<const MethodInfo> ArrayBuffer::GetMethods() const
{
  return{
    EP_MAKE_METHOD_EXPLICIT("Allocate", AllocateMethod, "Allocates an Array of the given type, size and length. Length may be a single value or a list of dimensions"),
  };
}

void ArrayBufferImpl::Allocate(SharedString _elementType, size_t _elementSize, Slice<const size_t> _shape)
{
  EPASSERT_THROW(_shape.length > 0, Result::InvalidArgument, "No dimensions given!");
  EPASSERT_THROW(_shape.length <= 4, Result::InvalidArgument, "More than 4 dimensional matrices is not supported...");

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
  bool alreadyAllocated = !pInstance->Empty();
  elementSize = _elementSize;
  pInstance->Buffer::Allocate(_elementSize*elements);
  if (alreadyAllocated)
    pInstance->Changed.Signal();

  // record element types as metadata
  Array<ElementMetadata,32> elementMetadata;
  size_t offset = 0;

  String et = elementType.trim();
  if(et.length > 2 && et.front() == '{' && et.back() == '}')
  {
    // multiple element stream
    et = et.slice(1, et.length-1).trim();
    et.tokenise([&](String t, size_t i) {
      t = t.trim();

      elementMetadata.pushBack(ElementMetadata{ nullptr, t, ElementInfo::Parse(t), offset });
      // calculate offset
      offset += GetElementTypeSize(t);
    }, ",");
  }
  else
    elementMetadata.pushBack(ElementMetadata{ nullptr, et, ElementInfo::Parse(et),  0 });

  pInstance->GetMetadata()->Insert("attributeinfo", elementMetadata);
}

bool ArrayBufferImpl::ReshapeInternal(Slice<const size_t> _shape, bool copy)
{
  // count total number of elements
  size_t elements = 1;
  for (size_t i = 0; i < _shape.length; ++i)
    elements *= _shape.ptr[i];

  // resize the buffer
  pInstance->Buffer::ResizeInternal(elements*elementSize, copy);
  if (copy)
  {
    // we only need to shuffle if the lesser significant dimensions changed size
    size_t shuffleDimensions = dimensions < _shape.length ? dimensions : _shape.length;
    bool needsShuffle = false;
    for (size_t i = 0; i < shuffleDimensions-1; ++i)
    {
      if (_shape.ptr[i] != shape[i])
      {
        needsShuffle = true;
        break;
      }
    }

    if (needsShuffle)
    {
      // if >1D array, and the dimensions changed, we need to shuffle the memory...
      EPASSERT(false, "TODO: support non-destructive resizing multi-dimensional arrays!");

      // TODO: if the 'x' dimension grew, then existing data needs to be interleaved with empty space
    }
  }

  // record new dimensions
  dimensions = _shape.length;
  for (size_t i = 0; i < _shape.length; ++i)
    shape[i] = _shape.ptr[i];

  return true; // TODO Error handling
}

} // namespace ep
