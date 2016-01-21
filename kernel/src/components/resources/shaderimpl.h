#pragma once
#ifndef EPSHADERIMPL_H
#define EPSHADERIMPL_H

#include "ep/cpp/component/resource/shader.h"

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
  void SetCode(SharedString _code) override final { code = _code; }

protected:
  EP_FRIENDS_WITH_IMPL(Material);
  friend class Renderer;
  friend class RenderShader;

  SharedString code;

  SharedPtr<RefCounted> spCachedShader;
};

} // namespace ep

#endif // EPSHADERIMPL_H
