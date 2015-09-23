#pragma once
#ifndef _UD_MODEL_H
#define _UD_MODEL_H

#include "components/resources/resource.h"
#include "components/resources/array.h"
#include "components/resources/material.h"
#include "ep/epsharedptr.h"
#include "ep/epstring.h"

namespace ud
{

SHARED_CLASS(RenderShaderProgram);
SHARED_CLASS(RenderVertexFormat);

PROTOTYPE_COMPONENT(Model);

class Model : public Resource
{
public:
  UD_COMPONENT(Model);

  MaterialRef GetMaterial() const { return spMaterial; }
  void SetMaterial(MaterialRef spMaterial) { this->spMaterial = spMaterial; }

  void SetVertexArray(ArrayBufferRef spVertices, udSlice<const udSharedString> attributeNames);
  void SetIndexArray(ArrayBufferRef spIndices) { this->spIndices = spIndices; }

protected:
  friend class GeomNode;

  Model(const ComponentDesc *pType, Kernel *pKernel, udSharedString uid, udInitParams initParams)
    : Resource(pType, pKernel, uid, initParams) {}

  void OnArrayChanged()
  {
    spRenderVertexFormat = nullptr;
  }

  RenderVertexFormatRef GetRenderVertexFormat(RenderShaderProgramRef spShaderProgram);

  struct VertexArray
  {
    ArrayBufferRef spArray;
    udFixedSlice<udSharedString, 1> attributes;
  };

  udFixedSlice<VertexArray, 2> vertexArrays;
  ArrayBufferRef spIndices;
  MaterialRef spMaterial;

  RenderVertexFormatRef spRenderVertexFormat = nullptr;
};

} // namespace ud

#endif // _UD_MODEL_H
