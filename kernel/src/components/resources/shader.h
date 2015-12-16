#pragma once
#ifndef _EP_SHADER_H
#define _EP_SHADER_H

#include "components/resources/resource.h"
#include "ep/cpp/sharedptr.h"

namespace ep
{

SHARED_CLASS(RenderShader);

SHARED_CLASS(Shader);

EP_ENUM(ShaderType,
        VertexShader,
        PixelShader);

class Shader : public Resource
{
  EP_DECLARE_COMPONENT(Shader, Resource, EPKERNEL_PLUGINVERSION, "Shader desc...")
public:

  SharedString GetCode() const { return code; }
  void SetCode(SharedString _code) { code = _code; }

protected:
  friend class Material;
  friend class RenderShader;

  Shader(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
    : Resource(pType, pKernel, uid, initParams) {}
  virtual ~Shader() {}

  RenderShaderRef GetRenderShader(int type);

  SharedString code;

  RenderShaderRef spRenderShader = nullptr;
};

} // namespace ep

#endif // _EP_SHADER_H
