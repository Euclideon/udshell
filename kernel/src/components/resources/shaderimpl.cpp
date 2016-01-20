#include "shaderimpl.h"
#include "renderresource.h"
#include "kernel.h"

namespace ep {

RenderShaderRef ShaderImpl::GetRenderShader(int type)
{
  if (!spRenderShader)
  {
    RenderShader *pShader = new RenderShader(GetKernel()->GetRenderer(), ShaderRef(pInstance), (epShaderType)type);
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

} // namespace ep
