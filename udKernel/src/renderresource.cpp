#include "renderscene.h"
#include "renderresource.h"
#include "hal/texture.h"
#include "hal/vertex.h"
#include "hal/shader.h"

namespace ud
{

static udArrayDataFormat GetElementType(udString type)
{
  static const struct TypeMap
  {
    const char *pName;
    udArrayDataFormat format;
  } s_typeMap[] = {
    { "f32[4]", udVDF_Float4 },
    { "f32[3]", udVDF_Float3 },
    { "f32[2]", udVDF_Float2 },
    { "f32[1]", udVDF_Float },
    { "f32", udVDF_Float },
    { "s32[4]", udVDF_Int4 },
    { "s32[3]", udVDF_Int3 },
    { "s32[2]", udVDF_Int2 },
    { "s32[1]", udVDF_Int },
    { "s32", udVDF_Int },
    { "u32[4]", udVDF_UInt4 },
    { "u32[3]", udVDF_UInt3 },
    { "u32[2]", udVDF_UInt2 },
    { "u32[1]", udVDF_UInt },
    { "u32", udVDF_UInt },
    { "s16[4]", udVDF_Short4 },
    { "s16[2]", udVDF_Short2 },
    { "s16", udVDF_Short },
    { "u16[4]", udVDF_UShort4 },
    { "u16[2]", udVDF_UShort2 },
    { "u16", udVDF_UShort },
    { "s8[4]", udVDF_Byte4N },
    { "u8[4]", udVDF_UByte4N_RGBA },
  };
  for (size_t i = 0; i<UDARRAYSIZE(s_typeMap); ++i)
  {
    if (type.eq(s_typeMap[i].pName))
      return s_typeMap[i].format;
  }
  return udVDF_Unknown;
}

RenderArray::RenderArray(Renderer *pRenderer, ArrayBufferRef spArrayBuffer, ArrayUsage usage)
  : RenderResource(pRenderer)
{
  udArrayDataFormat elements[16];
  size_t numElements = 0;

  udString type = spArrayBuffer->GetType();
  if (type[0] == '{')
    type = type.slice(1, type.length-1);

  udString element;
  while ((element = type.popToken(",")) != nullptr)
    elements[numElements++] = GetElementType(type);

  if (usage == ArrayUsage::VertexData)
  {
    pArray = udVertex_CreateVertexBuffer(elements, numElements);
  }
  else
  {
    UDASSERT(numElements == 1, "Index buffers may only have a single integer element!");
    pArray = udVertex_CreateIndexBuffer(elements[0]);
  }

  size_t size;
  const void *pData = spArrayBuffer->MapForRead(&size);
  udVertex_SetArrayBufferData(pArray, pData, size);
  spArrayBuffer->Unmap();
}
RenderArray::~RenderArray()
{
  udVertex_DestroyArrayBuffer(&pArray);
}

RenderTexture::RenderTexture(Renderer *pRenderer, ArrayBufferRef spArrayBuffer, TextureUsage usage)
  : RenderResource(pRenderer)
{
//  pTexture = udTexture_CreateTexture();
}
RenderTexture::~RenderTexture()
{
  udTexture_DestroyTexture(&pTexture);
}

RenderShader::RenderShader(Renderer *pRenderer, ShaderRef spShader, udShaderType type)
  : RenderResource(pRenderer), type(type)
{
  pShader = udShader_CreateShader(spShader->code.toStringz(), spShader->code.length, type);
}
RenderShader::~RenderShader()
{
  // TODO: write the destroy function! ;)
}

RenderShaderProgram::RenderShaderProgram(Renderer *pRenderer, RenderShaderRef vs, RenderShaderRef ps)
  : RenderResource(pRenderer)
{
  // link the shader
  pProgram = udShader_CreateShaderProgram(vs->pShader, ps->pShader);
  if (!pProgram)
    return;

  // TODO: derive KEY from 'this'
  uint32_t key = 0;
  pRenderer->shaderPrograms.Insert(key, this);
}
RenderShaderProgram::~RenderShaderProgram()
{
  // TODO: write the destroy function! ;)

  // TODO: derive KEY from 'this'
  uint32_t key = 0;
  pRenderer->shaderPrograms.Remove(key);
}
size_t RenderShaderProgram::numAttributes()
{
  return udShader_GetNumAttributes(pProgram);
}
udString RenderShaderProgram::getAttributeName(size_t i)
{
  return udShader_GetAttributeName(pProgram, i);
}
size_t RenderShaderProgram::numUniforms()
{
  return udShader_GetNumUniforms(pProgram);
}
udString RenderShaderProgram::getUniformName(size_t i)
{
  return udShader_GetUniformName(pProgram, i);
}
void RenderShaderProgram::setUniform(int i, const udFloat4 &v)
{
  udShader_SetCurrent(pProgram);
  udShader_SetProgramData(i, v);
}


RenderVertexFormat::RenderVertexFormat(Renderer *pRenderer, const udArrayElement *pElements, size_t numElements)
  : RenderResource(pRenderer)
{
  // TODO: derive KEY from 'this'
  uint32_t key = 0;
  pRenderer->vertexFormats.Insert(key, this);

  pFormat = udVertex_CreateFormatDeclaration(pElements, (int)numElements);
}
RenderVertexFormat::~RenderVertexFormat()
{
  udVertex_DestroyFormatDeclaration(&pFormat);

  // TODO: derive KEY from 'this'
  uint32_t key = 0;
  pRenderer->vertexFormats.Remove(key);
}

} // namespace ud
