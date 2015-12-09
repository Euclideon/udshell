#include "ep/cpp/component.h"
#include "material.h"
#include "renderresource.h"
#include "kernel.h"

namespace ep {

void Material::SetShader(ShaderType type, ShaderRef spShader)
{
  if (shaders[(int)type] == spShader)
    return;
  if (shaders[(int)type])
    shaders[(int)type]->Changed.Unsubscribe(Delegate<void()>(this, &Material::OnShaderChanged));
  shaders[(int)type] = spShader;
  if (spShader)
    spShader->Changed.Subscribe(Delegate<void()>(this, &Material::OnShaderChanged));
  OnShaderChanged();
}

void Material::SetMaterialProperty(SharedString property, const Float4 &val)
{
  properties.Insert(property, val);
}

void Material::OnShaderChanged()
{
  // remove shader properties from component
  // TODO...

  spRenderProgram = nullptr;

  // recreate the shader and populate properties
  GetRenderProgram();
}

void Material::SetRenderstate()
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

RenderShaderProgramRef Material::GetRenderProgram()
{
  if (!spRenderProgram)
  {
    RenderShaderRef spVS = shaders[0] ? shaders[0]->GetRenderShader(epST_VertexShader) : nullptr;
    RenderShaderRef spPS = shaders[1] ? shaders[1]->GetRenderShader(epST_PixelShader) : nullptr;
    if (spVS && spPS)
    {
      // TODO: check if this program already exists in `pRenderer->shaderPrograms`

      RenderShaderProgram *pProgram = new RenderShaderProgram(((kernel::Kernel&)GetKernel()).GetRenderer(), spVS, spPS);
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
