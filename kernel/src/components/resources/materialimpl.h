#pragma once
#ifndef EPMATERIALIMPL_H
#define EPMATERIALIMPL_H

#include "ep/cpp/component/resource/material.h"
#include "ep/cpp/internal/i/imaterial.h"
#include "ep/cpp/avltree.h"

namespace ep {

SHARED_CLASS(RenderShaderProgram);

class MaterialImpl : public BaseImpl<Material, IMaterial>
{
public:
  MaterialImpl(Component *pInstance, Variant::VarMap initParams)
    : ImplSuper(pInstance)
  {
  }

  ShaderRef GetShader(ShaderType type) const override final { return shaders[(int)type]; }
  void SetShader(ShaderType type, ShaderRef spShader) override final;

  ArrayBufferRef GetTexture(int index) const override final { return textures[index]; }
  void SetTexture(int index, ArrayBufferRef spArray) override final { textures[index] = spArray; }

  BlendMode GetBlendMode() const override final { return blendMode; }
  void SetBlendMode(BlendMode _blendMode) override final { blendMode = _blendMode; }

  CullMode GetCullMode() const override final { return cullMode; }
  void SetCullMode(CullMode _cullMode) override final { cullMode = _cullMode; }

  void SetMaterialProperty(SharedString property, const Float4 &val) { properties.Insert(property, val); }

protected:
  EP_FRIENDS_WITH_IMPL(GeomNode);

  ~MaterialImpl()
  {
    for (ShaderRef &s : shaders)
    {
      if (s)
        s->Changed.Unsubscribe(Delegate<void()>(this, &MaterialImpl::OnShaderChanged));
    }
  }

  void OnShaderChanged();
  void SetRenderstate();

  RenderShaderProgramRef GetRenderProgram();

  ShaderRef shaders[2];
  ArrayBufferRef textures[8];

  BlendMode blendMode = BlendMode::None;
  CullMode cullMode = CullMode::None;

  AVLTree<SharedString, Float4> properties;

  RenderShaderProgramRef spRenderProgram = nullptr;
};

} // namespace ep

#endif // EPMATERIALIMPL_H
