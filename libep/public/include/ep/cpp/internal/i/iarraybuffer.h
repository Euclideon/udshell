#pragma once
#ifndef _EP_IARRAYBUFFER_HPP
#define _EP_IARRAYBUFFER_HPP

#include "ep/cpp/component/component.h"

namespace ep {

class IArrayBuffer
{
public:
  virtual void Allocate(SharedString _elementType, size_t _elementSize, Slice<const size_t> _shape) = 0;

  virtual bool ReshapeInternal(Slice<const size_t> shape, bool copy) = 0;

  virtual SharedString GetElementType() const = 0;
  virtual size_t GetElementSize() const = 0;
  virtual size_t GetNumDimensions() const = 0;

  virtual size_t GetLength() const = 0;
  virtual Slice<const size_t> GetShape() const = 0;

  virtual Variant Save() const = 0;
};

} // namespace ep

#endif // _EP_IARRAYBUFFER_HPP
