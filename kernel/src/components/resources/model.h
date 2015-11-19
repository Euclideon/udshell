#pragma once
#ifndef _EP_MODEL_H
#define _EP_MODEL_H

#include "components/resources/resource.h"
#include "components/resources/array.h"
#include "components/resources/material.h"
#include "ep/cpp/sharedptr.h"

namespace kernel
{

SHARED_CLASS(RenderShaderProgram);
SHARED_CLASS(RenderVertexFormat);

PROTOTYPE_COMPONENT(Model);

class Model : public Resource
{
public:
  EP_COMPONENT(Model);

  MaterialRef GetMaterial() const { return spMaterial; }
  void SetMaterial(MaterialRef _spMaterial) { spMaterial = _spMaterial; }

  void SetVertexArray(ArrayBufferRef spVertices, Slice<const SharedString> attributeNames);
  void SetIndexArray(ArrayBufferRef _spIndices) { spIndices = _spIndices; }

protected:
  friend class GeomNode;

  Model(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, InitParams initParams)
    : Resource(pType, pKernel, uid, initParams) {}

  void OnArrayChanged()
  {
    spRenderVertexFormat = nullptr;
  }

  RenderVertexFormatRef GetRenderVertexFormat(RenderShaderProgramRef spShaderProgram);

  struct VertexArray
  {
    ArrayBufferRef spArray;
    Array<SharedString, 1> attributes;
  };

  Array<VertexArray, 2> vertexArrays;
  ArrayBufferRef spIndices;
  MaterialRef spMaterial;

  RenderVertexFormatRef spRenderVertexFormat = nullptr;
};

} // namespace kernel

#endif // _EP_MODEL_H
