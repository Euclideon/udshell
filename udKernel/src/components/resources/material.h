#pragma once
#ifndef _UD_MATERIAL_H
#define _UD_MATERIAL_H

#include "components/resources/resource.h"
#include "components/resources/shader.h"
#include "util/udsharedptr.h"
#include "util/udstring.h"
#include "util/udavltree.h"

namespace ud
{

SHARED_CLASS(ArrayBuffer);
SHARED_CLASS(RenderShaderProgram);

PROTOTYPE_COMPONENT(Material);

class Material : public Resource
{
public:
  UD_COMPONENT(Material);

  UD_ENUM(BlendMode,
          None,
          Alpha,
          Additive);

  UD_ENUM(CullMode,
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
  void SetBlendMode(BlendMode blendMode) { this->blendMode = blendMode; }

  CullMode GetCullMode() const { return cullMode; }
  void SetCullMode(CullMode cullMode) { this->cullMode = cullMode; }

  void SetMaterialProperty(udRCString property, const udFloat4 &val);

protected:
  friend class GeomNode;

  Material(const ComponentDesc *pType, Kernel *pKernel, udRCString uid, udInitParams initParams)
    : Resource(pType, pKernel, uid, initParams) {}
  virtual ~Material()
  {
    for (ShaderRef &s : shaders)
    {
      if (s)
        s->Changed.Unsubscribe(udDelegate<void()>(this, &Material::OnShaderChanged));
    }
  }

  void OnShaderChanged();
  void SetRenderstate();

  RenderShaderProgramRef GetRenderProgram();

  ShaderRef shaders[2];
  ArrayBufferRef textures[8];

  BlendMode blendMode = BlendMode::None;
  CullMode cullMode = CullMode::None;

  udAVLTree<udRCString, udFloat4> properties;

  RenderShaderProgramRef spRenderProgram = nullptr;
};

} // namespace ud

#endif // _UD_MATERIAL_H
