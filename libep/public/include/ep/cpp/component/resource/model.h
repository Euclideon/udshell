#pragma once
#ifndef _EP_MODEL_HPP
#define _EP_MODEL_HPP

#include "ep/cpp/component/resource/resource.h"
#include "ep/cpp/component/resource/arraybuffer.h"
#include "ep/cpp/internal/i/imodel.h"

namespace ep {

SHARED_CLASS(Model);

class Model : public Resource
{
  EP_DECLARE_COMPONENT_WITH_IMPL(Model, IModel, Resource, EPKERNEL_PLUGINVERSION, "Model desc...", 0)
public:

  MaterialRef GetMaterial() const { return pImpl->GetMaterial(); }
  void SetMaterial(MaterialRef spMaterial) { pImpl->SetMaterial(spMaterial); }

  void SetVertexArray(ArrayBufferRef spVertices, Slice<const SharedString> attributeNames) { pImpl->SetVertexArray(spVertices, attributeNames); }

  ArrayBufferRef GetIndexArray() const { return pImpl->GetIndexArray(); }
  void SetIndexArray(ArrayBufferRef spIndices) { pImpl->SetIndexArray(spIndices); }

protected:
  Model(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
    : Resource(pType, pKernel, uid, initParams)
  {
    pImpl = CreateImpl(initParams);
  }

private:
  Array<const PropertyInfo> GetProperties() const;
  Array<const MethodInfo> GetMethods() const;
};

} // namespace ep

#endif // _EP_MODEL_HPP
