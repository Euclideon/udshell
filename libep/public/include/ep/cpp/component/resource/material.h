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

class Material : public Resource, public IMaterial
{
  EP_DECLARE_COMPONENT_WITH_IMPL(Material, IMaterial, Resource, EPKERNEL_PLUGINVERSION, "Material Resource")
public:
  ShaderRef GetShader(ShaderType type) const override final { return pImpl->GetShader(type); }
  void SetShader(ShaderType type, ShaderRef spShader) override final { pImpl->SetShader(type, spShader); }

  ShaderRef GetVertexShader() const { return pImpl->GetShader(ShaderType::VertexShader); }
  void SetVertexShader(ShaderRef spShader) { pImpl->SetShader(ShaderType::VertexShader, spShader); }
  ShaderRef GetPixelShader() const { return pImpl->GetShader(ShaderType::PixelShader); }
  void SetPixelShader(ShaderRef spShader) { pImpl->SetShader(ShaderType::PixelShader, spShader); }

  ArrayBufferRef GetTexture(int index) const override final { return pImpl->GetTexture(index); }
  void SetTexture(int index, ArrayBufferRef spArray) override final { pImpl->SetTexture(index, spArray); }

  BlendMode GetBlendMode() const override final { return pImpl->GetBlendMode(); }
  void SetBlendMode(BlendMode blendMode) override final { pImpl->SetBlendMode(blendMode); }

  CullMode GetCullMode() const override final { return pImpl->GetCullMode(); }
  void SetCullMode(CullMode cullMode) override final { pImpl->SetCullMode(cullMode); }

  void SetMaterialProperty(SharedString property, const Float4 &val) override final { pImpl->SetMaterialProperty(property, val); }

protected:
  Material(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
    : Resource(pType, pKernel, uid, initParams)
  {
    pImpl = CreateImpl(initParams);
  }

  static Array<const PropertyInfo> GetProperties()
  {
    return{
      EP_MAKE_PROPERTY(VertexShader, "Vertex shader for rendering", nullptr, 0),
      EP_MAKE_PROPERTY(PixelShader, "Pixel shader for rendering", nullptr, 0),
      EP_MAKE_PROPERTY(BlendMode, "Frame buffer blend mode", nullptr, 0),
      EP_MAKE_PROPERTY(CullMode, "Back face cull mode", nullptr, 0),
    };
  }
  Array<const MethodInfo> GetMethods()
  {
    return{
      EP_MAKE_METHOD(GetTexture, "Gets the texture ArrayBuffer at the given index"),
      EP_MAKE_METHOD(SetTexture, "Sets the texture ArrayBuffer at the given index"),
      EP_MAKE_METHOD(SetMaterialProperty, "Set a Material Property"),
    };
  }
};

} // namespace ep

#endif // _EP_MATERIAL_HPP
