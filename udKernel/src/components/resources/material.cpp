#include "components/component.h"
#include "material.h"
#include "renderresource.h"
#include "kernel.h"

namespace ud
{

static CPropertyDesc props[] =
{
  {
    {
      "vertexshader", // id
      "Vertex Shader", // displayName
      "Vertex shader for rendering", // description
    },
    &Material::GetVertexShader,
    &Material::SetVertexShader,
  },
  {
    {
      "pixelshader", // id
      "Pixel Shader", // displayName
      "Pixel shader for rendering", // description
    },
    &Material::GetPixelShader,
    &Material::SetPixelShader,
  },
  {
    {
      "blendmode", // id
      "Blend Mode", // displayName
      "Frame buffer blend mode", // description
    },
    &Material::GetBlendMode,
    &Material::SetBlendMode,
  },
  {
    {
      "cullmode", // id
      "Cull Mode", // displayName
      "Back face cull mode", // description
    },
    &Material::GetCullMode,
    &Material::SetCullMode,
  }
};
/*
static CMethodDesc methods[] =
{
  {
    {
      "getshader",
      "Get a shader",
    },
    &Material::GetShader // method
  },
  {
    {
      "setshader",
      "Set a shader",
    },
    &Material::SetShader // method
  },
  {
    {
      "gettexture",
      "Get a texture",
    },
    &Material::GetTexture // method
  },
  {
    {
      "settexture",
      "Set a texture",
    },
    &Material::SetTexture // method
  }
};
*/
ComponentDesc Material::descriptor =
{
  &Resource::descriptor, // pSuperDesc

  UDSHELL_APIVERSION, // udVersion
  UDSHELL_PLUGINVERSION, // pluginVersion

  "material", // id
  "Material", // displayName
  "Material resource", // description

  udSlice<CPropertyDesc>(props, UDARRAYSIZE(props)), // properties
};

void Material::SetShader(ShaderType type, ShaderRef spShader)
{
  if (shaders[(int)type] == spShader)
    return;
  if (shaders[(int)type])
    shaders[(int)type]->Changed.Unsubscribe(udDelegate<void()>(this, &Material::OnShaderChanged));
  shaders[(int)type] = spShader;
  if (spShader)
    spShader->Changed.Subscribe(udDelegate<void()>(this, &Material::OnShaderChanged));
  OnShaderChanged();
}

void Material::SetMaterialProperty(udRCString property, const udFloat4 &val)
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
    udString name = spRenderProgram->getUniformName(i);
    const udFloat4 *pVal = properties.Get(name);
    if (pVal)
      spRenderProgram->setUniform((int)i, *pVal);
  }
}

RenderShaderProgramRef Material::GetRenderProgram()
{
  if (!spRenderProgram)
  {
    RenderShaderRef spVS = shaders[0] ? shaders[0]->GetRenderShader(udST_VertexShader) : nullptr;
    RenderShaderRef spPS = shaders[1] ? shaders[1]->GetRenderShader(udST_PixelShader) : nullptr;
    if (spVS && spPS)
    {
      // TODO: check if this program already exists in `pRenderer->shaderPrograms`

      RenderShaderProgram *pProgram = new RenderShaderProgram(pKernel->GetRenderer(), spVS, spPS);
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

} // namespace ud
