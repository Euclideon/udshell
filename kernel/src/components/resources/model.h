#pragma once
#ifndef _EP_MODEL_H
#define _EP_MODEL_H

#include "components/resources/resource.h"
#include "components/resources/array.h"
#include "components/resources/material.h"
#include "ep/epsharedptr.h"

namespace ep
{

SHARED_CLASS(RenderShaderProgram);
SHARED_CLASS(RenderVertexFormat);

PROTOTYPE_COMPONENT(Model);

class Model : public Resource
{
public:
  EP_COMPONENT(Model);

  MaterialRef GetMaterial() const { return spMaterial; }
  void SetMaterial(MaterialRef spMaterial) { this->spMaterial = spMaterial; }

  void SetVertexArray(ArrayBufferRef spVertices, epSlice<const epSharedString> attributeNames);
  void SetIndexArray(ArrayBufferRef spIndices) { this->spIndices = spIndices; }

protected:
  friend class GeomNode;

  Model(const ComponentDesc *pType, Kernel *pKernel, epSharedString uid, epInitParams initParams)
    : Resource(pType, pKernel, uid, initParams) {}

  void OnArrayChanged()
  {
    spRenderVertexFormat = nullptr;
  }

  RenderVertexFormatRef GetRenderVertexFormat(RenderShaderProgramRef spShaderProgram);

  struct VertexArray
  {
    ArrayBufferRef spArray;
    epArray<epSharedString, 1> attributes;
  };

  epArray<VertexArray, 2> vertexArrays;
  ArrayBufferRef spIndices;
  MaterialRef spMaterial;

  RenderVertexFormatRef spRenderVertexFormat = nullptr;
};

} // namespace ep

#endif // _EP_MODEL_H
