#include "ep/cpp/component/resource/model.h"
#include "modelimpl.h"
#include "renderresource.h"
#include "ep/cpp/kernel.h"
#include "materialimpl.h"

namespace ep {

Variant epToVariant(const RenderList &r)
{
  Variant::VarMap map;
  map.insert("type", r.type.v);
  map.insert("firstVertex", r.firstVertex);
  map.insert("firstIndex", r.firstIndex);
  map.insert("numVertices", r.numVertices);
  return std::move(map);
}

void epFromVariant(const Variant &variant, RenderList *r)
{
  r->type = PrimType::Type(variant["type"].asInt());
  r->firstVertex = variant["firstVertex"].as<size_t>();
  r->firstIndex = variant["firstIndex"].as<size_t>();
  r->numVertices = variant["numVertices"].as<size_t>();
}

Array<const PropertyInfo> Model::getProperties() const
{
  return{
    EP_MAKE_PROPERTY("material", GetMaterial, SetMaterial, "The Model's Material", nullptr, 0),
    EP_MAKE_PROPERTY("indexArray", GetIndexArray, SetIndexArray, "The Model's index buffer", nullptr, 0),
  };
}
Array<const MethodInfo> Model::getMethods() const
{
  return{
//    EP_MAKE_METHOD(SetVertexArray, "Set the Vertex buffer given an array of vertices and an array of attributes"), // TODO: Add this when we support passing slice to function
  };
}

void ModelImpl::AddVertexArray(ArrayBufferRef spVertices)
{
  EPTHROW_IF(!spVertices, Result::InvalidArgument, "array buffer was empty");

  for (size_t i = 0; i < vertexArrays.length; ++i)
  {
    if (vertexArrays[i] == spVertices)
      return;
  }

  vertexArrays = SharedArray<ArrayBufferRef>(Concat, vertexArrays, spVertices);
  OnArrayOrShaderChanged();
}

void ModelImpl::RemoveVertexArray(ArrayBufferRef spVertices)
{
  EPTHROW_IF(!spVertices, Result::InvalidArgument, "array buffer was empty");

  for (size_t i = 0; i < vertexArrays.length; ++i)
  {
    if (vertexArrays[i] == spVertices)
    {
      vertexArrays = SharedArray<ArrayBufferRef>(Concat, vertexArrays.slice(0, i), vertexArrays.slice(i + 1, vertexArrays.length));
      OnArrayOrShaderChanged();
      break;
    }
  }
}

ResourceRef ModelImpl::Clone() const
{
  ModelRef spNewModel = GetKernel()->CreateComponent<Model>();
  ModelImpl* pImpl = spNewModel->GetImpl<ModelImpl>();

  pImpl->vertexArrays = vertexArrays;
  pImpl->spIndices = spIndices;
  pImpl->spMaterial = spMaterial;
  pImpl->renderList = renderList;

  pImpl->spVertexFormatCache = spVertexFormatCache;

  return spNewModel;
}

} // namespace ep
