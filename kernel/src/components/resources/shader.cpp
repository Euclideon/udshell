#include "shader.h"
#include "renderresource.h"
#include "kernel.h"

namespace kernel
{

RenderShaderRef Shader::GetRenderShader(int type)
{
  if (!spRenderShader)
  {
    RenderShader *pShader = new RenderShader(GetKernel().GetRenderer(), ShaderRef(this), (epShaderType)type);
    if (!pShader->pShader)
    {
      delete pShader;
      spRenderShader = nullptr;
    }
    else
      spRenderShader = RenderShaderRef(pShader);
  }
  return spRenderShader;
}

} // namespace kernel
