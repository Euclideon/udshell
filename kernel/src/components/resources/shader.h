#pragma once
#ifndef _EP_SHADER_H
#define _EP_SHADER_H

#include "components/resources/resource.h"
#include "ep/epsharedptr.h"

namespace ep
{

SHARED_CLASS(RenderShader);

PROTOTYPE_COMPONENT(Shader);

EP_ENUM(ShaderType,
        VertexShader,
        PixelShader);

class Shader : public Resource
{
public:
  EP_COMPONENT(Shader);

  epSharedString GetCode() const { return code; }
  void SetCode(epSharedString code) { this->code = code; }

protected:
  friend class Material;
  friend class RenderShader;

  Shader(const ComponentDesc *pType, Kernel *pKernel, epSharedString uid, epInitParams initParams)
    : Resource(pType, pKernel, uid, initParams) {}
  virtual ~Shader() {}

  RenderShaderRef GetRenderShader(int type);

  epSharedString code;

  RenderShaderRef spRenderShader = nullptr;
};

} // namespace ep

#endif // _EP_SHADER_H
