#pragma once
#ifndef EPSHADERIMPL_H
#define EPSHADERIMPL_H

#include "ep/cpp/component/resource/shader.h"
#include "renderresource.h"

namespace ep {

SHARED_CLASS(RenderShader);


class ShaderImpl : public BaseImpl<Shader, IShader>
{
public:
  ShaderImpl(Component *pInstance, Variant::VarMap initParams)
    : ImplSuper(pInstance)
  {
  }

  SharedString GetCode() const override final { return code; }
  void SetCode(SharedString _code) override final { if (_code != code) { code = _code; spCachedShader = nullptr; } }

  ShaderType GetType() const override final { return type; }
  void SetType(ShaderType _type) override final { if (_type != type) { type = _type; spCachedShader = nullptr; } }

  SharedPtr<RefCounted> GetRenderShader() override final;

protected:
  EP_FRIENDS_WITH_IMPL(Material);
  friend class Renderer;
  friend class RenderShader;

  SharedString code;
  ShaderType type;
  RenderShaderRef spCachedShader;
};

} // namespace ep

#endif // EPSHADERIMPL_H
