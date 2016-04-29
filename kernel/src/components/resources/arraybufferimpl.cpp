#include "arraybufferimpl.h"
#include "renderresource.h"
#include "ep/cpp/kernel.h"
#include "ep/cpp/component/resource/metadata.h"

namespace ep {

Array<const PropertyInfo> ArrayBuffer::GetProperties() const
{
  return{
    EP_MAKE_PROPERTY_RO(ElementType, "Element type of the Array as a String", nullptr, 0),
    EP_MAKE_PROPERTY_RO(ElementSize, "The Array's element size", nullptr, 0),
    EP_MAKE_PROPERTY_RO(NumDimensions, "Number of dimensions", nullptr, 0),
    EP_MAKE_PROPERTY_RO(Length, "Number of Array elements", nullptr, 0),
    EP_MAKE_PROPERTY_RO(Shape, "The sizes of the Array's dimensions", nullptr, 0),
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
  bool alreadyAllocated = !pInstance->Empty();
  elementSize = _elementSize;
  pInstance->Buffer::Allocate(_elementSize*elements);
  if (alreadyAllocated)
    pInstance->Changed.Signal();

  // record element types as metadata
  Array<ElementMetadata,32> elementMetadata;
  int offset = 0;

  String et = _elementType.trim();
  if(et.length > 2 && et.front() == '{' && et.back() == '}')
  {
    // multiple element stream
    et = et.slice(1, et.length-1).trim();
    et.tokenise([&](String t, size_t i) {
      t = t.trim();

      // calculate offset
      elementMetadata.pushBack(ElementMetadata{ nullptr, t, offset });
      offset += GetElementTypeSize(t);
    }, ",");
  }
  else
    elementMetadata.pushBack(ElementMetadata{ nullptr, et, 0 });

  pInstance->GetMetadata()->Insert("attributeinfo", elementMetadata);
}

} // namespace ep
