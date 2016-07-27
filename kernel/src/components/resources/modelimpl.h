#pragma once
#ifndef _EP_MODELIMPL_HPP
#define _EP_MODELIMPL_HPP

#include "ep/cpp/component/resource/model.h"
#include "ep/cpp/component/resource/arraybuffer.h"
#include "ep/cpp/component/resource/material.h"
#include "ep/cpp/internal/i/imodel.h"
#include "hal/vertex.h"

namespace ep {

SHARED_CLASS(RenderShaderProgram);
SHARED_CLASS(RenderVertexFormat);

SHARED_CLASS(Model);

class ModelImpl : public BaseImpl<Model, IModel>
{
public:
  ModelImpl(Component *pInstance, Variant::VarMap initParams)
    : ImplSuper(pInstance), renderList(RenderList { PrimType::Triangles, 0, 0, 0 })
  {}

  MaterialRef GetMaterial() const override final { return spMaterial; }
  void SetMaterial(MaterialRef _spMaterial) override final
  {
    if (_spMaterial == spMaterial)
      return;

    if (spMaterial)
      spMaterial->changed.Unsubscribe(this, &ModelImpl::OnArrayOrShaderChanged);

    spMaterial = _spMaterial;

    if (spMaterial)
      spMaterial->changed.Subscribe(this, &ModelImpl::OnArrayOrShaderChanged);
  }

  void AddVertexArray(ArrayBufferRef spVertices) override final;
  void RemoveVertexArray(ArrayBufferRef spVertices) override final;
  Array<ArrayBufferRef> GetVertexArrays() override final { return vertexArrays; }

  ArrayBufferRef GetIndexArray() const override final { return spIndices; }
  void SetIndexArray(ArrayBufferRef _spIndices) override final { spIndices = _spIndices; }

  void SetRenderList(const RenderList& list) override final { renderList = list; }
  const RenderList& GetRenderList() const override final { return renderList; }

  ResourceRef Clone() const override final;

private:
  EP_FRIENDS_WITH_IMPL(GeomNode);
  void OnArrayOrShaderChanged() { spVertexFormatCache = nullptr; }

  void RetainShaderInputConfig(SharedPtr<RefCounted> sp) { spVertexFormatCache = sp; }

  Array<ArrayBufferRef, 2> vertexArrays;
  ArrayBufferRef spIndices;
  MaterialRef spMaterial;

  RenderList renderList;

  // TODO: spVertexFormatCache should be evicted when arrays are added/removed, material is changed, and also on the Resource::Changed events emitted by arrays and material.
  SharedPtr<RefCounted> spVertexFormatCache = nullptr;
};

} // namespace ep

#endif // _EP_MODELIMPL_HPP
