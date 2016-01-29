#pragma once
#if !defined(_EP_IMATERIAL_HPP)
#define _EP_IMATERIAL_HPP

#include "ep/cpp/component/component.h"

namespace ep {

SHARED_CLASS(ArrayBuffer);
SHARED_CLASS(Shader);

struct ShaderType;
struct BlendMode;
struct CullMode;

class IMaterial
{
public:
  virtual ShaderRef GetShader(ShaderType type) const = 0;
  virtual void SetShader(ShaderType type, ShaderRef spShader) = 0;

  virtual ArrayBufferRef GetTexture(int index) const = 0;
  virtual void SetTexture(int index, ArrayBufferRef spArray) = 0;

  virtual BlendMode GetBlendMode() const = 0;
  virtual void SetBlendMode(BlendMode blendMode) = 0;

  virtual CullMode GetCullMode() const = 0;
  virtual void SetCullMode(CullMode cullMode) = 0;

  virtual void SetMaterialProperty(SharedString property, const Float4 &val) = 0;
};

} // namespace ep

#endif // _EP_IMATERIAL_HPP
