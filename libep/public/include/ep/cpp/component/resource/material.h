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

  ResourceRef clone() const override { return pImpl->Clone(); }

  // Material Properties
  Variant getMaterialProperty(String property) const { return pImpl->GetMaterialProperty(property); }
  void setMaterialProperty(String property, Variant val) { pImpl->SetMaterialProperty(property, val); }

  // Shaders
  ShaderRef getShader(ShaderType type) const { return getMaterialProperty(s_shaderNames[type]).as<ShaderRef>(); }
  void setShader(ShaderType type, ShaderRef spShader) { setMaterialProperty(s_shaderNames[type], spShader); }
  void setShader(ShaderType type, String shader) { setMaterialProperty(s_shaderNames[type], shader); }

  ShaderRef getVertexShader() const { return getMaterialProperty(s_shaderNames[ShaderType::VertexShader]).as<ShaderRef>(); }
  void setVertexShader(ShaderRef spShader) { EPTHROW_IF(!spShader || spShader->getType() != ShaderType::VertexShader, Result::InvalidArgument, "Not a vertex shader"); setMaterialProperty(s_shaderNames[ShaderType::VertexShader], spShader); }
  void setVertexShader(String shader) { setShader(ShaderType::VertexShader, shader); }

  ShaderRef getPixelShader() const { return getMaterialProperty(s_shaderNames[ShaderType::PixelShader]).as<ShaderRef>(); }
  void setPixelShader(ShaderRef spShader) { EPTHROW_IF(!spShader || spShader->getType() != ShaderType::PixelShader, Result::InvalidArgument, "Not a pixel shader");  setMaterialProperty(s_shaderNames[ShaderType::PixelShader], spShader); }
  void setPixelShader(String shader) { setShader(ShaderType::PixelShader, shader); }

  ShaderRef getGeometryShader() const { return getMaterialProperty(s_shaderNames[ShaderType::GeometryShader]).as<ShaderRef>(); }
  void setGeometryShader(ShaderRef spShader) { EPTHROW_IF(!spShader || spShader->getType() != ShaderType::GeometryShader, Result::InvalidArgument, "Not a Geometry shader");  setMaterialProperty(s_shaderNames[ShaderType::GeometryShader], spShader); }
  void setGeometryShader(String shader) { setShader(ShaderType::GeometryShader, shader); }

  ShaderRef getTesselationControlShader() const { return getMaterialProperty(s_shaderNames[ShaderType::TesselationControlShader]).as<ShaderRef>(); }
  void setTesselationControlShader(ShaderRef spShader) { EPTHROW_IF(!spShader || spShader->getType() != ShaderType::TesselationControlShader, Result::InvalidArgument, "Not a TesselationControl shader");  setMaterialProperty(s_shaderNames[ShaderType::TesselationControlShader], spShader); }
  void setTesselationControlShader(String shader) { setShader(ShaderType::TesselationControlShader, shader); }

  ShaderRef getTesselationEvaluationShader() const { return getMaterialProperty(s_shaderNames[ShaderType::TesselationEvaluationShader]).as<ShaderRef>(); }
  void setTesselationEvaluationShader(ShaderRef spShader) { EPTHROW_IF(!spShader || spShader->getType() != ShaderType::TesselationEvaluationShader, Result::InvalidArgument, "Not a TesselationEvaluation shader");   setMaterialProperty(s_shaderNames[ShaderType::TesselationEvaluationShader], spShader); }
  void setTesselationEvaluationShader(String shader) { setShader(ShaderType::TesselationEvaluationShader, shader); }

  // Other states
  BlendMode getBlendMode() const { return getMaterialProperty("blendMode").as<BlendMode>(); }
  void setBlendMode(BlendMode blendMode)  { setMaterialProperty("blendMode", blendMode); }

  CullMode getCullMode() const { return getMaterialProperty("cullMode").as<CullMode>(); }
  void setCullMode(CullMode cullMode) { setMaterialProperty("cullMode", cullMode); }

  CompareFunc getDepthCompareFunc() const { return getMaterialProperty("depthFunc").as<CompareFunc>(); }
  void setDepthCompareFunc(CompareFunc func) { setMaterialProperty("depthFunc", func); }

  StencilState getStencilState() const { return getMaterialProperty("stencilState").as<StencilState>(); }
  void setStencilState(const StencilState &state) { return setMaterialProperty("stencilState", state); }

  StencilState getFrontStencilState() const { return getMaterialProperty("frontStencilState").as<StencilState>(); }
  void setFrontStencilState(const StencilState &state) { return setMaterialProperty("frontStencilState", state); }

  StencilState getBackStencilState() const { return getMaterialProperty("backStencilState").as<StencilState>(); }
  void setBackStencilState(const StencilState &state) { return setMaterialProperty("backStencilState", state); }

//  // Textures
//  ArrayBufferRef GetTexture(int index) const { return GetMaterialProperty(index); }
//  void SetTexture(int index, ArrayBufferRef spArray) { SetMaterialProperty(index, spArray); }

#if 0 // TODO: When ranges are implemented
  const ShaderPropertyRange& getShaderUniformsRange() const { return pImpl->GetShaderUniformsRange(); }
  const ShaderPropertyRange& getShaderAttributesRange() const { return pImpl->GetShaderAttributesRange(); }
#endif

protected:
  Material(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
    : Resource(pType, pKernel, uid, initParams)
  {
    pImpl = createImpl(initParams);
  }

private:
  void setVertexShader_Meta(Variant shader) { setMaterialProperty(s_shaderNames[ShaderType::VertexShader], shader); }
  void setPixelShader_Meta(Variant shader) { setMaterialProperty(s_shaderNames[ShaderType::PixelShader], shader); }
  void setGeometryShader_Meta(Variant shader) { setMaterialProperty(s_shaderNames[ShaderType::GeometryShader], shader); }
  void setTesselationControlShader_Meta(Variant shader) { setMaterialProperty(s_shaderNames[ShaderType::TesselationControlShader], shader); }
  void setTesselationEvaluationShader_Meta(Variant shader) { setMaterialProperty(s_shaderNames[ShaderType::TesselationEvaluationShader], shader); }

  const PropertyDesc *getPropertyDesc(String _name, EnumerateFlags enumerateFlags = 0) const override { return pImpl->GetPropertyDesc(_name, enumerateFlags); }

  Array<const PropertyInfo> getProperties() const;
  Array<const MethodInfo> getMethods() const;

  static const char * const s_shaderNames[];
  static AVLTree<String, int> s_builtinProperties;
};

} // namespace ep

#endif // _EP_MATERIAL_HPP
