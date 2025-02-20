#pragma once
#if !defined(_EP_SHADER_HPP)
#define _EP_SHADER_HPP

#include "ep/cpp/component/resource/resource.h"
#include "ep/cpp/internal/i/ishader.h"

namespace ep {

SHARED_CLASS(Shader);

EP_ENUM(ShaderType,
  VertexShader = 0,
  PixelShader,
  GeometryShader,
  TesselationControlShader,
  TesselationEvaluationShader,
  ComputeShader
);

class Shader : public Resource
{
  EP_DECLARE_COMPONENT_WITH_IMPL(ep, Shader, IShader, Resource, EPKERNEL_PLUGINVERSION, "Shader desc...", 0)
public:
  SharedString getCode() const { return pImpl->GetCode(); }
  void setCode(SharedString code) { pImpl->SetCode(code); }

  void setType(ShaderType type) { pImpl->SetType(type); }
  ShaderType getType() const { return pImpl->GetType(); }

  SharedPtr<RefCounted> getRenderShader() { return pImpl->GetRenderShader(); }

protected:
  Shader(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
    : Resource(pType, pKernel, uid, initParams)
  {
    pImpl = createImpl(initParams);
  }

  Array<const PropertyInfo> getProperties() const;
};

} // namespace ep

#endif // _EP_SHADER_HPP
