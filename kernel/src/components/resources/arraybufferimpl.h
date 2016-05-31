#pragma once
#ifndef _EP_ARRAYBUFFERIMPL_HPP
#define _EP_ARRAYBUFFERIMPL_HPP

#include "ep/cpp/component/resource/arraybuffer.h"
#include "ep/cpp/internal/i/iarraybuffer.h"

#include <utility>

namespace ep {

SHARED_CLASS(RenderResource);
SHARED_CLASS(Scene);

SHARED_CLASS(ArrayBuffer);

class ArrayBufferImpl : public BaseImpl<ArrayBuffer, IArrayBuffer>
{
public:
  ArrayBufferImpl(Component *_pInstance, Variant::VarMap initParams)
    : ImplSuper(_pInstance)
  {
  }

  void Allocate(SharedString _elementType, size_t _elementSize, Slice<const size_t> _shape) override final;

  bool ReshapeInternal(Slice<const size_t> shape, bool copy) override final;

  SharedString GetElementType() const override final { return elementType; }
  size_t GetElementSize() const override final { return elementSize; }
  size_t GetNumDimensions() const override final { return dimensions; }

  size_t GetLength() const override final
  {
    size_t elements = shape[0];
    for (size_t i = 1; i<dimensions; ++i)
      elements *= shape[i];
    return elements;
  }
  Slice<const size_t> GetShape() const override final
  {
    return Slice<const size_t>(shape, dimensions);
  }

  Variant Save() const override final { return pInstance->Super::Save(); }

protected:
  friend class Renderer;
  friend class GeomNode;

  SharedString elementType;
  size_t elementSize;
  size_t dimensions;
  size_t shape[4];
};

} // namespace ep

#endif // _EP_ARRAYBUFFERIMPL_HPP
