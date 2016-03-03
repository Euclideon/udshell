#include "arraybufferimpl.h"
#include "renderresource.h"
#include "ep/cpp/kernel.h"

namespace ep {

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
}

} // namespace ep
