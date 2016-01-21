#include "ep/cpp/component/component.h"
#include "materialimpl.h"
#include "shaderimpl.h"
#include "renderresource.h"
#include "kernel.h"

namespace ep {

void MaterialImpl::SetShader(ShaderType type, ShaderRef spShader)
{
  if (shaders[(int)type] == spShader)
    return;
  if (shaders[(int)type])
    shaders[(int)type]->Changed.Unsubscribe(Delegate<void()>(this, &MaterialImpl::OnShaderChanged));
  shaders[(int)type] = spShader;
  if (spShader)
    spShader->Changed.Subscribe(Delegate<void()>(this, &MaterialImpl::OnShaderChanged));
  OnShaderChanged();
}

void MaterialImpl::OnShaderChanged()
{
  // remove shader properties from component
  // TODO...

  spRenderProgram = nullptr;

  // recreate the shader and populate properties
  GetRenderProgram();
}

// TODO: this isn't used anywhere...
void MaterialImpl::SetRenderstate()
{
  if (!spRenderProgram)
    return;

  size_t numUniforms = spRenderProgram->numUniforms();
  for (size_t i = 0; i < numUniforms; ++i)
  {
    const Float4 *pVal = properties.Get(spRenderProgram->getUniformName(i));
    if (pVal)
      spRenderProgram->setUniform((int)i, *pVal);
  }
}

RenderShaderProgramRef MaterialImpl::GetRenderProgram()
{
  if (!spRenderProgram)
  {
    RenderShaderRef spVS = shaders[0] ? shaders[0]->GetImpl<ShaderImpl>()->GetRenderShader(epST_VertexShader) : nullptr;
    RenderShaderRef spPS = shaders[1] ? shaders[1]->GetImpl<ShaderImpl>()->GetRenderShader(epST_PixelShader) : nullptr;
    if (spVS && spPS)
    {
      // TODO: check if this program already exists in `pRenderer->shaderPrograms`

      RenderShaderProgram *pProgram = new RenderShaderProgram(GetKernel()->GetRenderer(), spVS, spPS);
      if (!pProgram->pProgram)
      {
        delete pProgram;
        spRenderProgram = nullptr;
      }
      else
        spRenderProgram = RenderShaderProgramRef(pProgram);

      // populate the material with properties from the shader...
      // TODO...
    }
  }
  return spRenderProgram;
}

} // namespace ep
