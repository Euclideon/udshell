#pragma once
#ifndef _EP_MODEL_H
#define _EP_MODEL_H

#include "ep/cpp/component/resource/resource.h"
#include "ep/cpp/component/resource/arraybuffer.h"
#include "components/resources/materialimpl.h"
#include "ep/cpp/sharedptr.h"

namespace ep
{

SHARED_CLASS(RenderShaderProgram);
SHARED_CLASS(RenderVertexFormat);

SHARED_CLASS(Model);

class Model : public Resource
{
  EP_DECLARE_COMPONENT(Model, Resource, EPKERNEL_PLUGINVERSION, "Model desc...")
public:

  MaterialRef GetMaterial() const { return spMaterial; }
  void SetMaterial(MaterialRef _spMaterial) { spMaterial = _spMaterial; }

  void SetVertexArray(ArrayBufferRef spVertices, Slice<const SharedString> attributeNames);
  void SetIndexArray(ArrayBufferRef _spIndices) { spIndices = _spIndices; }

protected:
  friend class GeomNode;

  Model(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
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

  static Array<const PropertyInfo> GetProperties()
  {
    return{
      EP_MAKE_PROPERTY(Material, "The Model's Material", nullptr, 0),
      EP_MAKE_PROPERTY_WO(IndexArray, "The Model's index buffer", nullptr, 0),
    };
  }
  static Array<const MethodInfo> GetMethods()
  {
    return{
      //EP_MAKE_METHOD(SetVertexArray, "Set the Vertex buffer given an array of vertices and an array of attributes"), // TODO: Add this when we support passing slice to function
    };
  }
};

} // namespace ep

#endif // _EP_MODEL_H
