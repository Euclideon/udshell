#pragma once
#ifndef _UD_ARRAYBUFFER_H
#define _UD_ARRAYBUFFER_H

#include "components/resources/buffer.h"
#include "util/udsharedptr.h"
#include "util/udstring.h"

namespace ud
{

SHARED_CLASS(ArrayBuffer);

class ArrayBuffer : public Buffer
{
public:
  UD_COMPONENT(ArrayBuffer);

  void Allocate(size_t elementSize, size_t length)
  {
    Buffer::Allocate(elementSize*length);
    this->elementSize = elementSize;
    dimensions = 1;
    shape[0] = length;
  }
  void Allocate(size_t elementSize, udSlice<const size_t> shape)
  {
    UDASSERT(shape.length > 0, "No dimensions given!");
    UDASSERT(shape.length <= 4, "More than 4 dimensional matrices is not supported...");
    size_t elements = shape[0];
    for (size_t i = 1; i<shape.length; ++i)
      elements *= shape[i];
    Buffer::Allocate(elementSize*elements);
    this->elementSize = elementSize;
    dimensions = shape.length;
    for (size_t i = 0; i<shape.length; ++i)
      this->shape[0] = shape[i];
  }

  size_t GetElementSize() const { return elementSize; }
  size_t GetNumDimensions() const { return dimensions; }

  size_t GetLength() const
  {
    size_t elements = shape[0];
    for (size_t i = 1; i<dimensions; ++i)
      elements *= shape[i];
  }
  udSlice<const size_t> GetShape() const
  {
    return udSlice<const size_t>(shape, dimensions);
  }

protected:
  ArrayBuffer(const ComponentDesc *pType, Kernel *pKernel, udRCString uid, udInitParams initParams)
    : Buffer(pType, pKernel, uid, initParams) {}
  virtual ~ArrayBuffer();

  void Allocate(size_t size) = delete;

  size_t elementSize;
  size_t dimensions;
  size_t shape[4];

  // element type description...
};

} // namespace ud

#endif // _UD_ARRAYBUFFER_H
