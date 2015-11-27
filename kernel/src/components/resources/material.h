#pragma once
#ifndef _EP_MATERIAL_H
#define _EP_MATERIAL_H

#include "components/resources/resource.h"
#include "components/resources/shader.h"
#include "ep/cpp/sharedptr.h"
#include "ep/cpp/avltree.h"

namespace kernel
{

SHARED_CLASS(ArrayBuffer);
SHARED_CLASS(RenderShaderProgram);

PROTOTYPE_COMPONENT(Material);

class Material : public Resource
{
  EP_DECLARE_COMPONENT(Material, Resource, EPKERNEL_PLUGINVERSION, "Material resource")
public:

  EP_ENUM(BlendMode,
          None,
          Alpha,
          Additive);

  EP_ENUM(CullMode,
          None,
          CW,
          CCW);

  ShaderRef GetShader(ShaderType type) const { return shaders[(int)type]; }
  void SetShader(ShaderType type, ShaderRef spShader);

  ShaderRef GetVertexShader() const { return GetShader(ShaderType::VertexShader); }
  void SetVertexShader(ShaderRef spShader) { SetShader(ShaderType::VertexShader, spShader); }
  ShaderRef GetPixelShader() const { return GetShader(ShaderType::PixelShader); }
  void SetPixelShader(ShaderRef spShader) { SetShader(ShaderType::PixelShader, spShader); }

  ArrayBufferRef GetTexture(int index) const { return textures[index]; }
  void SetTexture(int index, ArrayBufferRef spArray) { textures[index] = spArray; }

  BlendMode GetBlendMode() const { return blendMode; }
  void SetBlendMode(BlendMode _blendMode) { blendMode = _blendMode; }

  CullMode GetCullMode() const { return cullMode; }
  void SetCullMode(CullMode _cullMode) { cullMode = _cullMode; }

  void SetMaterialProperty(SharedString property, const Float4 &val);

protected:
  friend class GeomNode;

  Material(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, InitParams initParams)
    : Resource(pType, pKernel, uid, initParams) {}
  virtual ~Material()
  {
    for (ShaderRef &s : shaders)
    {
      if (s)
        s->Changed.Unsubscribe(Delegate<void()>(this, &Material::OnShaderChanged));
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

  static Array<const PropertyInfo> GetProperties()
  {
    return{
      EP_MAKE_PROPERTY(VertexShader, "Vertex shader for rendering", nullptr, 0),
      EP_MAKE_PROPERTY(PixelShader, "Pixel shader for rendering", nullptr, 0),
      EP_MAKE_PROPERTY(BlendMode, "Frame buffer blend mode", nullptr, 0),
      EP_MAKE_PROPERTY(CullMode, "Back face cull mode", nullptr, 0),
    };
  }
};

} // namespace kernel

#endif // _EP_MATERIAL_H
