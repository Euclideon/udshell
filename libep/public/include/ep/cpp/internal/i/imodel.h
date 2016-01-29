#pragma once
#ifndef _EP_IMODEL_HPP
#define _EP_IMODEL_HPP

#include "ep/cpp/component/resource/material.h"

namespace ep {

SHARED_CLASS(ArrayBuffer);

class IModel
{
public:
  virtual MaterialRef GetMaterial() const = 0;
  virtual void SetMaterial(MaterialRef spMaterial) = 0;

  virtual void SetVertexArray(ArrayBufferRef spVertices, Slice<const SharedString> attributeNames) = 0;

  virtual ArrayBufferRef GetIndexArray() const = 0;
  virtual void SetIndexArray(ArrayBufferRef spIndices) = 0;
};

} // namespace ep

#endif // _EP_IMODEL_HPP
