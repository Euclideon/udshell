#include "renderscene.h"
#include "renderresource.h"
#include "hal/texture.h"
#include "hal/vertex.h"
#include "hal/shader.h"
#include "components/resources/shaderimpl.h"
#include "ep/cpp/component/resource/metadata.h"

namespace ep {

epArrayDataFormat GetElementType(String type, int *pSize = nullptr)
{
  static const struct TypeMap
  {
    const char *pName;
    epArrayDataFormat format;
    int size;
  } s_typeMap[] = {
    { "f32[4]", epVDF_Float4,      16 },
    { "f32[3]", epVDF_Float3,      12 },
    { "f32[2]", epVDF_Float2,       8 },
    { "f32[1]", epVDF_Float,        4 },
    { "f32",    epVDF_Float,        4 },
    { "s32[4]", epVDF_Int4,        16 },
    { "s32[3]", epVDF_Int3,        12 },
    { "s32[2]", epVDF_Int2,         8 },
    { "s32[1]", epVDF_Int,          4 },
    { "s32",    epVDF_Int,          4 },
    { "u32[4]", epVDF_UInt4,       16 },
    { "u32[3]", epVDF_UInt3,       12 },
    { "u32[2]", epVDF_UInt2,        8 },
    { "u32[1]", epVDF_UInt,         4 },
    { "u32",    epVDF_UInt,         4 },
    { "s16[4]", epVDF_Short4,       8 },
    { "s16[2]", epVDF_Short2,       4 },
    { "s16",    epVDF_Short,        2 },
    { "u16[4]", epVDF_UShort4,      8 },
    { "u16[2]", epVDF_UShort2,      4 },
    { "u16",    epVDF_UShort,       2 },
    { "s8[4]",  epVDF_Byte4N,       4 },
    { "u8[4]",  epVDF_UByte4N_RGBA, 4 }
  }; // TODO: Make this a map and Add all the extra types later
  for (size_t i = 0; i<EPARRAYSIZE(s_typeMap); ++i)
  {
    if (type.eq(s_typeMap[i].pName))
    {
      if (pSize)
        *pSize = s_typeMap[i].size;
      return s_typeMap[i].format;
    }
  }
  return epVDF_Unknown;
}

// TODO: Remove this horribleness once s_typeMap is a map.
int GetElementTypeSize(String type)
{
  int size = 0;
  GetElementType(type, &size);
  return size;
}

RenderArray::RenderArray(Renderer *pRenderer, ArrayBufferRef spArrayBuffer, ArrayUsage usage)
  : RenderResource(pRenderer)
{
  SharedArray<ElementMetadata> attribs = spArrayBuffer->GetMetadata()->Get("attributeinfo").as<SharedArray<ElementMetadata>>();

  Array<epArrayDataFormat> elements;
  for (const auto &elem : attribs)
    elements.pushBack(GetElementType(elem.type));

  if (usage == ArrayUsage::VertexData)
  {
    pArray = epVertex_CreateVertexBuffer(elements.ptr, elements.length);
  }
  else
  {
    EPASSERT(elements.length == 1, "Index buffers may only have a single integer element!");
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

RenderTexture::RenderTexture(Renderer *pRenderer, ArrayBufferRef spArrayBuffer)
  : RenderResource(pRenderer)
{
  auto shape = spArrayBuffer->GetShape();

  // TODO: we need to support cubemap, and array textures here (additional args?)
  switch (shape.length)
  {
    case 1: usage = epTT_1D; break;
    case 2: usage = epTT_2D; break;
    case 3: usage = epTT_3D; break;
    default:
      EPASSERT(false, "TODO: array, cube textures");
  }

  // TODO: support texture formats in a better-er way...
  SharedString type = spArrayBuffer->GetElementType();
  if (type.eq("u32")) format = epIF_BGRA8; // this makes the assumption that images of type uint32_t are BGRA (ie, as output by UD)
  else if (type.eq("u8[4]")) format = epIF_RGBA8; // array of bytes is interpreted as {R,G,B,A}, just like arrays of anything
  else if (type.eq("f32")) format = epIF_R_F32;
  else EPASSERT(false, "TODO: better system for handling texture formats!");

  // attempt to map the buffer
  Slice<const void> colorBuffer = spArrayBuffer->MapForRead();
  epscope(exit) { spArrayBuffer->Unmap(); };

  // copy the data into the texture
  pTexture = epTexture_CreateTexture(usage, shape[0], shape[1], 1, format);
  if (pTexture)
    epTexture_SetImageData(pTexture, -1, 0, colorBuffer.ptr);
  else
    EPTHROW_ERROR(epR_Failure, "Failed to create texture!");
}
RenderTexture::~RenderTexture()
{
  if(pTexture)
    epTexture_DestroyTexture(&pTexture);
}

RenderShader::RenderShader(Renderer *pRenderer, SharedString code, epShaderType type)
  : RenderResource(pRenderer), type(type)
{
  pShader = epShader_CreateShader(code.toStringz(), code.length, type);
}
RenderShader::~RenderShader()
{
  epShader_DestroyShader(&pShader);
}

RenderShaderProgram::RenderShaderProgram(Renderer *pRenderer, Slice<RenderShaderRef> shaders)
  : RenderResource(pRenderer)
{
  Array<epShader*, ShaderType::ComputeShader+1> halShaders;
  for (RenderShaderRef rs : shaders)
    halShaders.pushBack(rs->pShader);

  // link the shader
  pProgram = epShader_CreateShaderProgram(halShaders.ptr, halShaders.length);
  EPTHROW_IF_NULL(pProgram, epR_Failure, "Failed to compile shader program");
}

RenderShaderProgram::~RenderShaderProgram()
{
  epShader_DestroyShaderProgram(&pProgram);
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

epShaderElement RenderShaderProgram::getAttributeType(size_t i)
{
  return epShader_GetAttributeType(pProgram, i);
}

epShaderElement RenderShaderProgram::getUniformType(size_t i)
{
  return epShader_GetUniformType(pProgram, i);
}

SharedString RenderShaderProgram::getAttributeTypeString(size_t i)
{
  epShaderElement type = epShader_GetAttributeType(pProgram, i);
  char typeBuffer[16];
  size_t length;
  epShader_GetElementTypeString(type, typeBuffer, sizeof(typeBuffer), &length);

  return SharedString(typeBuffer, length);
}

SharedString RenderShaderProgram::getUniformTypeString(size_t i)
{
  epShaderElement type = epShader_GetUniformType(pProgram, i);
  char typeBuffer[16];
  size_t length;
  epShader_GetElementTypeString(type, typeBuffer, sizeof(typeBuffer), &length);

  return SharedString(typeBuffer, length);
}

template <typename T>
inline Variant GetShaderElement(epShaderProgram *pProgram, size_t param)
{
  T data;
  epShader_GetProgramData(pProgram, param, &data);
  return data;
}

Variant RenderShaderProgram::getUniform(size_t i)
{
  epShaderElement t = epShader_GetUniformType(pProgram, i);
  EPASSERT(t.type >= epSET_Int && t.type <= epSET_Double, "Invalid type");
  if (t.m == 1)
  {
    switch (t.n)
    {
      case 1:
        switch (t.type)
        {
          case epSET_Int:
            return GetShaderElement<int>(pProgram, i);
          case epSET_Uint:
            return GetShaderElement<uint32_t>(pProgram, i);
          case epSET_Float:
            return GetShaderElement<float>(pProgram, i);
          case epSET_Double:
            return GetShaderElement<double>(pProgram, i);
        }
        break;
      case 2:
        switch (t.type)
        {
          case epSET_Int:
            return GetShaderElement<Vector2<int>>(pProgram, i);
          case epSET_Uint:
            return GetShaderElement<Vector2<uint32_t>>(pProgram, i);
          case epSET_Float:
            return GetShaderElement<Vector2<float>>(pProgram, i);
          case epSET_Double:
            return GetShaderElement<Vector2<double>>(pProgram, i);
        }
        break;
      case 3:
        switch (t.type)
        {
          case epSET_Int:
            return GetShaderElement<Vector3<int>>(pProgram, i);
          case epSET_Uint:
            return GetShaderElement<Vector3<uint32_t>>(pProgram, i);
          case epSET_Float:
            return GetShaderElement<Vector3<float>>(pProgram, i);
          case epSET_Double:
            return GetShaderElement<Vector3<double>>(pProgram, i);
        }
        break;
      case 4:
        switch (t.type)
        {
          case epSET_Int:
            return GetShaderElement<Vector4<int>>(pProgram, i);
          case epSET_Uint:
            return GetShaderElement<Vector4<uint32_t>>(pProgram, i);
          case epSET_Float:
            return GetShaderElement<Vector4<float>>(pProgram, i);
          case epSET_Double:
            return GetShaderElement<Vector4<double>>(pProgram, i);
        }
      default:
        EPTHROW(epR_Failure, "vector length {0} not supported", t.n);
        break;
    }
  }
  else if (t.m == 4)
  {
    if (t.n == 4)
    {
      switch (t.type)
      {
        case epSET_Int:
        case epSET_Uint:
          EPTHROW(epR_Failure, "Integer Matrix types not supported.");
        case epSET_Float:
          return GetShaderElement<Matrix4x4<float>>(pProgram, i);
        case epSET_Double:
          return GetShaderElement<Matrix4x4<double>>(pProgram, i);
      }
    }
    else
    {
      EPTHROW(epR_Failure, "Matrix {0},{1} not supported", t.m, t.n);
    }
  }
  else
  {
    EPTHROW(epR_Failure, "Unsupported element type m {0}, n {1}, type {2}", t.m, t.n, t.type);
  }

  return Variant();
}

void RenderShaderProgram::Use()
{
  epShader_SetCurrent(pProgram);
}

void RenderShaderProgram::setUniform(size_t i, Variant v)
{
  epShaderElement t = epShader_GetUniformType(pProgram, i);
  epShaderElementType et = (epShaderElementType)t.type;
  EPASSERT(et >= epSET_Int && et <= epSET_Double, "Invalid type");

  if (t.m == 1)
  {
    switch (t.n)
    {
      case 1:
      {
        switch (et)
        {
          case epSET_Int:
            epShader_SetProgramData(i, v.as<int>());
            return;
          case epSET_Uint:
            epShader_SetProgramData(i, v.as<uint32_t>());
            return;
          case epSET_Float:
            epShader_SetProgramData(i, v.as<float>());
            return;
          case epSET_Double:
            epShader_SetProgramData(i, v.as<double>());
            return;
        }
      }
      case 2:
      {
        switch (et)
        {
          case epSET_Int:
            epShader_SetProgramData(i, v.as<Vector2<int>>());
            return;
          case epSET_Uint:
            epShader_SetProgramData(i, v.as<Vector2<uint32_t>>());
            return;
          case epSET_Float:
            epShader_SetProgramData(i, v.as<Vector2<float>>());
            return;
          case epSET_Double:
            epShader_SetProgramData(i, v.as<Vector2<double>>());
            return;
        }
      }
      case 3:
      {
        switch (t.type)
        {
        case epSET_Int:
          epShader_SetProgramData(i, v.as<Vector3<int>>());
          return;
        case epSET_Uint:
          epShader_SetProgramData(i, v.as<Vector3<uint32_t>>());
          return;
        case epSET_Float:
          epShader_SetProgramData(i, v.as<Vector3<float>>());
          return;
        case epSET_Double:
          epShader_SetProgramData(i, v.as<Vector3<double>>());
          return;
        }
      }
      case 4:
      {
        switch (t.type)
        {
        case epSET_Int:
          epShader_SetProgramData(i, v.as<Vector4<int>>());
          return;
        case epSET_Uint:
          epShader_SetProgramData(i, v.as<Vector4<uint32_t>>());
          return;
        case epSET_Float:
          epShader_SetProgramData(i, v.as<Vector4<float>>());
          return;
        case epSET_Double:
          epShader_SetProgramData(i, v.as<Vector4<double>>());
          return;
        }
      }
      default:
        EPTHROW(epR_Failure, "vector length {0} not supported", t.n);
        break;
    }
  }
  else if (t.m == 4)
  {
    if (t.n == 4)
    {
      switch (t.type)
      {
        case epSET_Int:
        case epSET_Uint:
          EPTHROW(epR_Failure, "Integer Matrix types not supported.");
        case epSET_Float:
          epShader_SetProgramData(i, v.as<Matrix4x4<float>>());
          return;
        case epSET_Double:
          epShader_SetProgramData(i, v.as<Matrix4x4<double>>());
          return;
      }
    }
    else
    {
      EPTHROW(epR_Failure, "Matrix {0},{1} not supported", t.m, t.n);
    }
  }
  else
  {
    EPTHROW(epR_Failure, "Unsupported element type m {0}, n {1}, type {2}", t.m, t.n, t.type);
  }
}

RenderShaderInputConfig::RenderShaderInputConfig(Renderer *pRenderer, SharedArray<ArrayBufferRef> vertexArrays, RenderShaderProgramRef spProgram)
  : RenderResource(pRenderer)
{
  Array<epArrayElement, 8> elementArray;
  for (size_t i = 0; i < vertexArrays.length; ++i)
  {
    Variant varAttribs = vertexArrays[i]->GetMetadata()->Get("attributeinfo");
    EPASSERT_THROW(varAttribs.isValid(), epR_Failure, "attribute info not present in buffer metadata");
    SharedArray<ElementMetadata> attribs = varAttribs.as<SharedArray<ElementMetadata>>();

    size_t stride = attribs.back().offset + GetElementTypeSize(attribs.back().type);
    for (const auto &elem : attribs)
    {
      epArrayElement &halElem = elementArray.pushBack();
      memcpy(halElem.attributeName, elem.name.ptr, Min(elem.name.length, sizeof(halElem.attributeName) - 1));
      halElem.attributeName[sizeof(halElem.attributeName) - 1] = '\0';

      halElem.format = GetElementType(elem.type);
      halElem.stream = (int)i;
      halElem.offset = (int)elem.offset;
      halElem.stride = (int)stride;
    }
  }

  pConfig = epVertex_CreateShaderInputConfig(elementArray.ptr, (int)elementArray.length, spProgram->pProgram);
}

RenderShaderInputConfig::~RenderShaderInputConfig()
{
  epVertex_DestroyShaderInputConfig(&pConfig);
}

SharedArray<int> RenderShaderInputConfig::GetActiveStreams()
{
  Array<int, 0> streams(Alloc, 32);
  int numStreams = 0;
  epVertex_GetShaderInputConfigStreams(pConfig, streams.ptr, streams.length, &numStreams);
  streams.resize(numStreams);
  return streams;
}

} // namespace ep
