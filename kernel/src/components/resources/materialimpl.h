#pragma once
#ifndef EPMATERIALIMPL_H
#define EPMATERIALIMPL_H

#include "ep/cpp/component/resource/material.h"
#include "ep/cpp/internal/i/imaterial.h"
#include "ep/cpp/avltree.h"
#include "hal/shader.h"

namespace ep {

SHARED_CLASS(RenderShaderProgram);

EP_ENUM(TriangleFace,
  Default = 0,
  Front,
  Back
);

struct ShaderProperty
{
  SharedString type;
  Variant data;
  union
  {
    epShaderElement element;
    uint64_t elemData;
  };

  size_t uniformIndex;

  bool current;
  bool dirty;
};

using ShaderPropertyMap = AVLTree<SharedString, ShaderProperty>;

class MaterialImpl : public BaseImpl<Material, IMaterial>
{
public:
  MaterialImpl(Component *pInstance, Variant::VarMap initParams)
    :  ImplSuper(pInstance) {}

  const PropertyDesc *GetPropertyDesc(String _name, EnumerateFlags enumerateFlags = 0) const override final;

  Variant GetMaterialProperty(String property) const override final;
  void SetMaterialProperty(String property, Variant data) override final;

  ResourceRef Clone() const override final;

  ShaderRef GetShader(ShaderType type) const { return shaders[(int)type]; }
  void SetShader(ShaderType type, ShaderRef spShader);

  BlendMode GetBlendMode() const { return blendMode; }
  void SetBlendMode(BlendMode _blendMode) { blendMode = _blendMode; }

  CullMode GetCullMode() const { return cullMode; }
  void SetCullMode(CullMode _cullMode) { cullMode = _cullMode; }

  CompareFunc GetDepthCompareFunc() const { return depthCompareFunc; }
  void SetDepthCompareFunc(CompareFunc func) { depthCompareFunc = func;}

  StencilState GetStencilState(TriangleFace face) const { return stencilStates[face]; }
  void SetStencilState(TriangleFace face, const StencilState &state) { stencilStates[face] = state; }

  ArrayBufferRef GetTexture(int index) const { return textures[index]; }
  void SetTexture(int index, ArrayBufferRef spArray) { textures[index] = spArray; }

  const ShaderPropertyMap& GetShaderUniforms() const { return uniforms; }
  const ShaderPropertyMap& GetShaderAttributes() const { return attributes; }

protected:
  EP_FRIENDS_WITH_IMPL(GeomNode);
  EP_FRIENDS_WITH_IMPL(Scene);
  friend class Renderer;

  ~MaterialImpl()
  {
    for (ShaderRef &s : shaders)
    {
      if (s)
        s->changed.unsubscribe(Delegate<void()>(this, &MaterialImpl::OnShaderChanged));
    }
  }

  class DynamicPropertyData : public RefCounted
  {
  public:
    DynamicPropertyData(SharedString name) : name(name) {}
    SharedString name;
  };
  Variant Get(Slice<const Variant> args, const RefCounted &data) const { return GetMaterialProperty(((const DynamicPropertyData&)data).name); }
  Variant Set(Slice<const Variant> args, const RefCounted &data) { SetMaterialProperty(((const DynamicPropertyData&)data).name, args[0]); return Variant(); }

  void OnShaderChanged();
  void BuildShaderProgram();
  void PopulateShaderProperties();
  void InvalidateProperties();

  static const int NumShaders = ShaderType::ComputeShader+1;

  ShaderRef shaders[NumShaders];
  Array<ArrayBufferRef, 1> textures;

  // TODO: Change stencilStates to store just front and back and then make setting the default state set both.
  StencilState stencilStates[TriangleFace::Back+1];
  BlendMode blendMode = BlendMode::Alpha;
  CullMode cullMode = CullMode::CCW;
  CompareFunc depthCompareFunc = CompareFunc::LessEqual;

  ShaderPropertyMap uniforms;
  ShaderPropertyMap attributes;

  SharedPtr<RenderShaderProgram> spShaderProgram;

  mutable AVLTree<SharedString, PropertyDesc> dynamicPropertyCache;

  static AVLTree<String, int> builtinProperties;
};

} // namespace ep

#endif // EPMATERIALIMPL_H
