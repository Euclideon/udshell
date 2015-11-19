#include "shader.h"
#include "renderresource.h"
#include "kernel.h"

namespace kernel
{

ComponentDesc Shader::descriptor =
{
  &Resource::descriptor, // pSuperDesc

  EPSHELL_APIVERSION, // epVersion
  EPSHELL_PLUGINVERSION, // pluginVersion

  "shader", // id
  "Shader", // displayName
  "Shader resource", // description
};

RenderShaderRef Shader::GetRenderShader(int type)
{
  if (!spRenderShader)
  {
    RenderShader *pShader = new RenderShader(pKernel->GetRenderer(), ShaderRef(this), (epShaderType)type);
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
