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

  virtual void AddVertexArray(ArrayBufferRef spVertices) = 0;
  virtual void RemoveVertexArray(ArrayBufferRef spVertices) = 0;
  virtual SharedArray<ArrayBufferRef> GetVertexArrays() = 0;

  virtual ArrayBufferRef GetIndexArray() const = 0;
  virtual void SetIndexArray(ArrayBufferRef spIndices) = 0;

  virtual void SetRenderList(const RenderList& list) = 0;
  virtual const RenderList& GetRenderList() const = 0;
};

} // namespace ep

#endif // _EP_IMODEL_HPP
