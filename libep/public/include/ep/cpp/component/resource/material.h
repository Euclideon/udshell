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

// TODO: Expose all properties/methods to meta
// TODO: Add enums for each of these and move this structure to render.h
// TODO: Consider how to set _EXT texture settings
struct TextureSampler
{
  int minFilter;
  int maxFilter;

  int textureMinLOD;
  int textureMaxLOD;

  int wrapS;
  int wrapT;
  int wrapR;

  Float4 borderColor;

  int compareMode;
  int compareFunc;

  int clampToEdge;

  float anisotropy = 1.0f;
  ArrayBufferRef texture;
};

class Material : public Resource
{
  EP_DECLARE_COMPONENT_WITH_IMPL(ep, Material, IMaterial, Resource, EPKERNEL_PLUGINVERSION, "Material Resource", 0)
public:

  // Material Properties
  Variant GetMaterialProperty(String property) const { return pImpl->GetMaterialProperty(property); }
  void SetMaterialProperty(String property, Variant val) { pImpl->SetMaterialProperty(property, val); }

  // Shaders
  ShaderRef GetShader(ShaderType type) const { return GetMaterialProperty(s_shaderNames[type]).as<ShaderRef>(); }
  void SetShader(ShaderType type, ShaderRef spShader) { SetMaterialProperty(s_shaderNames[type], spShader); }

  ShaderRef GetVertexShader() const { return GetMaterialProperty(s_shaderNames[ShaderType::VertexShader]).as<ShaderRef>(); }
  void SetVertexShader(ShaderRef spShader) { EPTHROW_IF(!spShader || spShader->GetType() != ShaderType::VertexShader, Result::InvalidArgument, "Not a vertex shader"); SetMaterialProperty(s_shaderNames[ShaderType::VertexShader], spShader); }

  ShaderRef GetPixelShader() const { return GetMaterialProperty(s_shaderNames[ShaderType::PixelShader]).as<ShaderRef>(); }
  void SetPixelShader(ShaderRef spShader) { EPTHROW_IF(!spShader || spShader->GetType() != ShaderType::PixelShader, Result::InvalidArgument, "Not a pixel shader");  SetMaterialProperty(s_shaderNames[ShaderType::PixelShader], spShader); }

  ShaderRef GetGeometryShader() const { return GetMaterialProperty(s_shaderNames[ShaderType::GeometryShader]).as<ShaderRef>(); }
  void SetGeometryShader(ShaderRef spShader) { EPTHROW_IF(!spShader || spShader->GetType() != ShaderType::GeometryShader, Result::InvalidArgument, "Not a Geometry shader");  SetMaterialProperty(s_shaderNames[ShaderType::GeometryShader], spShader); }

  ShaderRef GetTesselationControlShader() const { return GetMaterialProperty(s_shaderNames[ShaderType::TesselationControlShader]).as<ShaderRef>(); }
  void SetTesselationControlShader(ShaderRef spShader) { EPTHROW_IF(!spShader || spShader->GetType() != ShaderType::TesselationControlShader, Result::InvalidArgument, "Not a TesselationControl shader");  SetMaterialProperty(s_shaderNames[ShaderType::TesselationControlShader], spShader); }

  ShaderRef GetTesselationEvaluationShader() const { return GetMaterialProperty(s_shaderNames[ShaderType::TesselationEvaluationShader]).as<ShaderRef>(); }
  void SetTesselationEvaluationShader(ShaderRef spShader) { EPTHROW_IF(!spShader || spShader->GetType() != ShaderType::TesselationEvaluationShader, Result::InvalidArgument, "Not a TesselationEvaluation shader");   SetMaterialProperty(s_shaderNames[ShaderType::TesselationEvaluationShader], spShader); }

  // Other states
  BlendMode GetBlendMode() const { return GetMaterialProperty("blendmode").as<BlendMode>(); }
  void SetBlendMode(BlendMode blendMode)  { SetMaterialProperty("blendmode", blendMode); }

  CullMode GetCullMode() const { return GetMaterialProperty("cullmode").as<CullMode>(); }
  void SetCullMode(CullMode cullMode) { SetMaterialProperty("cullmode", cullMode); }

  CompareFunc GetDepthCompareFunc() const { return GetMaterialProperty("depthfunc").as<CompareFunc>(); }
  void SetDepthCompareFunc(CompareFunc func) { SetMaterialProperty("depthfunc", func); }

  StencilState GetStencilState() const { return GetMaterialProperty("stencilstate").as<StencilState>(); }
  void SetStencilState(const StencilState &state) { return SetMaterialProperty("stencilstate", state); }

  StencilState GetFrontStencilState() const { return GetMaterialProperty("frontstencilstate").as<StencilState>(); }
  void SetFrontStencilState(const StencilState &state) { return SetMaterialProperty("frontstencilstate", state); }

  StencilState GetBackStencilState() const { return GetMaterialProperty("backstencilstate").as<StencilState>(); }
  void SetBackStencilState(const StencilState &state) { return SetMaterialProperty("backstencilstate", state); }

//  // Textures
//  ArrayBufferRef GetTexture(int index) const { return GetMaterialProperty(index); }
//  void SetTexture(int index, ArrayBufferRef spArray) { SetMaterialProperty(index, spArray); }

#if 0 // TODO: When ranges are implemented
  const ShaderPropertyRange& GetShaderUniformsRange() const { return pImpl->GetShaderUniformsRange(); }
  const ShaderPropertyRange& GetShaderAttributesRange() const { return pImpl->GetShaderAttributesRange(); }
#endif

protected:
  Material(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
    : Resource(pType, pKernel, uid, initParams)
  {
    pImpl = CreateImpl(initParams);
  }

  const PropertyDesc *GetPropertyDesc(String _name, EnumerateFlags enumerateFlags = 0) const override { return pImpl->GetPropertyDesc(_name, enumerateFlags); }

  Array<const PropertyInfo> GetProperties();
  Array<const MethodInfo> GetMethods();

  static const char * const s_shaderNames[];
  static AVLTree<String, int> s_builtinProperties;
};

} // namespace ep

#endif // _EP_MATERIAL_HPP
