#include "renderscene.h"
#include "renderresource.h"
#include "hal/texture.h"
#include "hal/vertex.h"
#include "hal/shader.h"

namespace ep
{

static epArrayDataFormat GetElementType(String type)
{
  static const struct TypeMap
  {
    const char *pName;
    epArrayDataFormat format;
  } s_typeMap[] = {
    { "f32[4]", epVDF_Float4 },
    { "f32[3]", epVDF_Float3 },
    { "f32[2]", epVDF_Float2 },
    { "f32[1]", epVDF_Float },
    { "f32", epVDF_Float },
    { "s32[4]", epVDF_Int4 },
    { "s32[3]", epVDF_Int3 },
    { "s32[2]", epVDF_Int2 },
    { "s32[1]", epVDF_Int },
    { "s32", epVDF_Int },
    { "u32[4]", epVDF_UInt4 },
    { "u32[3]", epVDF_UInt3 },
    { "u32[2]", epVDF_UInt2 },
    { "u32[1]", epVDF_UInt },
    { "u32", epVDF_UInt },
    { "s16[4]", epVDF_Short4 },
    { "s16[2]", epVDF_Short2 },
    { "s16", epVDF_Short },
    { "u16[4]", epVDF_UShort4 },
    { "u16[2]", epVDF_UShort2 },
    { "u16", epVDF_UShort },
    { "s8[4]", epVDF_Byte4N },
    { "u8[4]", epVDF_UByte4N_RGBA },
  };
  for (size_t i = 0; i<UDARRAYSIZE(s_typeMap); ++i)
  {
    if (type.eq(s_typeMap[i].pName))
      return s_typeMap[i].format;
  }
  return epVDF_Unknown;
}

RenderArray::RenderArray(Renderer *pRenderer, ArrayBufferRef spArrayBuffer, ArrayUsage usage)
  : RenderResource(pRenderer)
{
  epArrayDataFormat elements[16];
  size_t numElements = 0;

  String type = spArrayBuffer->GetType();
  if (type[0] == '{')
    type = type.slice(1, type.length-1);

  String element;
  while ((element = type.popToken(",")) != nullptr)
    elements[numElements++] = GetElementType(type);

  if (usage == ArrayUsage::VertexData)
  {
    pArray = epVertex_CreateVertexBuffer(elements, numElements);
  }
  else
  {
    EPASSERT(numElements == 1, "Index buffers may only have a single integer element!");
    pArray = epVertex_CreateIndexBuffer(elements[0]);
  }

  Slice<const void> data = spArrayBuffer->MapForRead();
  epVertex_SetArrayBufferData(pArray, data.ptr, data.length);
  spArrayBuffer->Unmap();
}
RenderArray::~RenderArray()
{
  epVertex_DestroyArrayBuffer(&pArray);
}

RenderTexture::RenderTexture(Renderer *pRenderer, ArrayBufferRef spArrayBuffer, TextureUsage usage)
  : RenderResource(pRenderer)
{
//  pTexture = epTexture_CreateTexture();
}
RenderTexture::~RenderTexture()
{
  epTexture_DestroyTexture(&pTexture);
}

RenderShader::RenderShader(Renderer *pRenderer, ShaderRef spShader, epShaderType type)
  : RenderResource(pRenderer), type(type)
{
  pShader = epShader_CreateShader(spShader->code.toStringz(), spShader->code.length, type);
}
RenderShader::~RenderShader()
{
  // TODO: write the destroy function! ;)
}

RenderShaderProgram::RenderShaderProgram(Renderer *pRenderer, RenderShaderRef vs, RenderShaderRef ps)
  : RenderResource(pRenderer)
{
  // link the shader
  pProgram = epShader_CreateShaderProgram(vs->pShader, ps->pShader);
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
  return epShader_GetNumAttributes(pProgram);
}
String RenderShaderProgram::getAttributeName(size_t i)
{
  return epShader_GetAttributeName(pProgram, i);
}
size_t RenderShaderProgram::numUniforms()
{
  return epShader_GetNumUniforms(pProgram);
}
String RenderShaderProgram::getUniformName(size_t i)
{
  return epShader_GetUniformName(pProgram, i);
}
void RenderShaderProgram::setUniform(int i, const Float4 &v)
{
  epShader_SetCurrent(pProgram);
  epShader_SetProgramData(i, v);
}


RenderVertexFormat::RenderVertexFormat(Renderer *pRenderer, const epArrayElement *pElements, size_t numElements)
  : RenderResource(pRenderer)
{
  // TODO: derive KEY from 'this'
  uint32_t key = 0;
  pRenderer->vertexFormats.Insert(key, this);

  pFormat = epVertex_CreateFormatDeclaration(pElements, (int)numElements);
}
RenderVertexFormat::~RenderVertexFormat()
{
  epVertex_DestroyFormatDeclaration(&pFormat);

  // TODO: derive KEY from 'this'
  uint32_t key = 0;
  pRenderer->vertexFormats.Remove(key);
}

} // namespace ep
