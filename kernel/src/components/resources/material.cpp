#include "components/component.h"
#include "material.h"
#include "renderresource.h"
#include "kernel.h"

namespace ep
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

  EPSHELL_APIVERSION, // epVersion
  EPSHELL_PLUGINVERSION, // pluginVersion

  "material", // id
  "Material", // displayName
  "Material resource", // description

  Slice<CPropertyDesc>(props, UDARRAYSIZE(props)), // properties
};

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
    String name = spRenderProgram->getUniformName(i);
    const Float4 *pVal = properties.Get(name);
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

} // namespace ep
