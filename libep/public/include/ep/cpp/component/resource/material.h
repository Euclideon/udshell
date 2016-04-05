#pragma once
#if !defined(_EP_MATERIAL_HPP)
#define _EP_MATERIAL_HPP

#include "ep/cpp/internal/i/imaterial.h"
#include "ep/cpp/component/resource/resource.h"
#include "ep/cpp/component/resource/shader.h"
#include "ep/cpp/component/resource/arraybuffer.h"
#include "ep/cpp/render.h"

namespace ep {

SHARED_CLASS(Material);

class Material : public Resource
{
  EP_DECLARE_COMPONENT_WITH_IMPL(ep, Material, IMaterial, Resource, EPKERNEL_PLUGINVERSION, "Material Resource", 0)
public:
  ShaderRef GetShader(ShaderType type) const { return pImpl->GetShader(type); }
  void SetShader(ShaderType type, ShaderRef spShader) { pImpl->SetShader(type, spShader); }

  ShaderRef GetVertexShader() const { return pImpl->GetShader(ShaderType::VertexShader); }
  void SetVertexShader(ShaderRef spShader) { pImpl->SetShader(ShaderType::VertexShader, spShader); }
  ShaderRef GetPixelShader() const { return pImpl->GetShader(ShaderType::PixelShader); }
  void SetPixelShader(ShaderRef spShader) { pImpl->SetShader(ShaderType::PixelShader, spShader); }

  ArrayBufferRef GetTexture(int index) const { return pImpl->GetTexture(index); }
  void SetTexture(int index, ArrayBufferRef spArray) { pImpl->SetTexture(index, spArray); }

  BlendMode GetBlendMode() const { return pImpl->GetBlendMode(); }
  void SetBlendMode(BlendMode blendMode) { pImpl->SetBlendMode(blendMode); }

  CullMode GetCullMode() const { return pImpl->GetCullMode(); }
  void SetCullMode(CullMode cullMode) { pImpl->SetCullMode(cullMode); }

  void SetMaterialProperty(SharedString property, const Float4 &val) { pImpl->SetMaterialProperty(property, val); }

protected:
  Material(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
    : Resource(pType, pKernel, uid, initParams)
  {
    pImpl = CreateImpl(initParams);
  }

private:
  Array<const PropertyInfo> GetProperties() const;
  Array<const MethodInfo> GetMethods() const;
};

} // namespace ep

#endif // _EP_MATERIAL_HPP
