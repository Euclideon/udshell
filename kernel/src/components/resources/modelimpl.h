#pragma once
#ifndef _EP_MODELIMPL_HPP
#define _EP_MODELIMPL_HPP

#include "ep/cpp/component/resource/model.h"
#include "ep/cpp/component/resource/arraybuffer.h"
#include "ep/cpp/component/resource/material.h"
#include "ep/cpp/internal/i/imodel.h"

namespace ep {

SHARED_CLASS(RenderShaderProgram);
SHARED_CLASS(RenderVertexFormat);

SHARED_CLASS(Model);

class ModelImpl : public BaseImpl<Model, IModel>
{
public:
  ModelImpl(Component *pInstance, Variant::VarMap initParams)
    : ImplSuper(pInstance)
  {}

  MaterialRef GetMaterial() const override final { return spMaterial; }
  void SetMaterial(MaterialRef _spMaterial) override final { spMaterial = _spMaterial; }

  void SetVertexArray(ArrayBufferRef spVertices, Slice<const SharedString> attributeNames) override final;

  ArrayBufferRef GetIndexArray() const override final { return spIndices; }
  void SetIndexArray(ArrayBufferRef _spIndices) override final { spIndices = _spIndices; }

private:
  void KeepCached(SharedPtr<RefCounted> spRC) { spVertexFormatCache = spRC; }
  void OnArrayOrShaderChanged() { spVertexFormatCache = nullptr; }

  Array<VertexArray, 4> vertexArrays;
  ArrayBufferRef spIndices;
  MaterialRef spMaterial;

  SharedPtr<RefCounted> spVertexFormatCache = nullptr;
};

} // namespace ep

#endif // _EP_MODELIMPL_HPP
