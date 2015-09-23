#pragma once
#ifndef _UD_SHADER_H
#define _UD_SHADER_H

#include "components/resources/resource.h"
#include "ep/epsharedptr.h"
#include "ep/epstring.h"

namespace ud
{

SHARED_CLASS(RenderShader);

PROTOTYPE_COMPONENT(Shader);

UD_ENUM(ShaderType,
        VertexShader,
        PixelShader);

class Shader : public Resource
{
public:
  UD_COMPONENT(Shader);

  udSharedString GetCode() const { return code; }
  void SetCode(udSharedString code) { this->code = code; }

protected:
  friend class Material;
  friend class RenderShader;

  Shader(const ComponentDesc *pType, Kernel *pKernel, udSharedString uid, udInitParams initParams)
    : Resource(pType, pKernel, uid, initParams) {}
  virtual ~Shader() {}

  RenderShaderRef GetRenderShader(int type);

  udSharedString code;

  RenderShaderRef spRenderShader = nullptr;
};

} // namespace ud

#endif // _UD_SHADER_H
