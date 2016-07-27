#pragma once
#ifndef _EP_MODEL_HPP
#define _EP_MODEL_HPP

#include "ep/cpp/component/resource/resource.h"
#include "ep/cpp/component/resource/arraybuffer.h"
#include "ep/cpp/internal/i/imodel.h"

namespace ep {

SHARED_CLASS(Model);
// TODO: Expose all properties/methods to meta
class Model : public Resource
{
  EP_DECLARE_COMPONENT_WITH_IMPL(ep, Model, IModel, Resource, EPKERNEL_PLUGINVERSION, "Model desc...", 0)
public:

  ResourceRef clone() const override { return pImpl->Clone(); }

  MaterialRef GetMaterial() const { return pImpl->GetMaterial(); }
  void SetMaterial(MaterialRef spMaterial) { pImpl->SetMaterial(spMaterial); }

  void AddVertexArray(ArrayBufferRef spVertices) { pImpl->AddVertexArray(spVertices); }
  void RemoveVertexArray(ArrayBufferRef spVertices) { pImpl->RemoveVertexArray(spVertices); }
  Array<ArrayBufferRef> GetVertexArrays() { return pImpl->GetVertexArrays(); }

  ArrayBufferRef GetIndexArray() const { return pImpl->GetIndexArray(); }
  void SetIndexArray(ArrayBufferRef spIndices) { pImpl->SetIndexArray(spIndices); }

  void SetRenderList(const RenderList &list) { pImpl->SetRenderList(list); }
  const RenderList& GetRenderList() const { return pImpl->GetRenderList(); }

protected:
  Model(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
    : Resource(pType, pKernel, uid, initParams)
  {
    pImpl = createImpl(initParams);
  }

private:
  Array<const PropertyInfo> getProperties() const;
  Array<const MethodInfo> getMethods() const;
};

} // namespace ep

#endif // _EP_MODEL_HPP
