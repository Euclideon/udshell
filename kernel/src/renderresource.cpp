#include "renderscene.h"
#include "renderresource.h"
#include "components/resources/shaderimpl.h"
#include "ep/cpp/component/resource/metadata.h"
#include "hal/texture.h"
#include "hal/vertex.h"
#include "hal/shader.h"

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
  SharedArray<ElementMetadata> attribs = spArrayBuffer->getMetadata()->get("attributeInfo").as<SharedArray<ElementMetadata>>();

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

  Slice<const void> data = spArrayBuffer->mapForRead();
  epVertex_SetArrayBufferData(pArray, data.ptr, data.length);
  spArrayBuffer->unmap();
}
RenderArray::~RenderArray()
{
  epVertex_DestroyArrayBuffer(&pArray);
}

RenderTexture::RenderTexture(Renderer *pRenderer, ArrayBufferRef spArrayBuffer)
  : RenderResource(pRenderer)
{
  auto shape = spArrayBuffer->getShape();

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
  SharedString type = spArrayBuffer->getElementType();
  if (type.eq("u32")) format = epIF_BGRA8; // this makes the assumption that images of type uint32_t are BGRA (ie, as output by UD)
  else if (type.eq("u8[4]")) format = epIF_RGBA8; // array of bytes is interpreted as {R,G,B,A}, just like arrays of anything
  else if (type.eq("f32")) format = epIF_R_F32;
  else EPASSERT(false, "TODO: better system for handling texture formats!");

  // attempt to map the buffer
  Slice<const void> colorBuffer = spArrayBuffer->mapForRead();
  epscope(exit) { spArrayBuffer->unmap(); };

  // copy the data into the texture
  pTexture = epTexture_CreateTexture(usage, shape[0], shape[1], 1, format);
  if (pTexture)
    epTexture_SetImageData(pTexture, -1, 0, colorBuffer.ptr);
  else
    EPTHROW_ERROR(Result::Failure, "Failed to create texture!");
}
RenderTexture::~RenderTexture()
{
  if(pTexture)
    epTexture_DestroyTexture(&pTexture);
}

RenderConstantBuffer::RenderConstantBuffer(Renderer *pRenderer, BufferRef spBuffer)
  : RenderResource(pRenderer)
{
  Slice<const void> src = spBuffer->mapForRead();
  EPASSERT_THROW(src, Result::Failure, "Failed to Map Buffer for Reading");
  epscope(exit) { spBuffer->unmap(); };

  pBuffer = epAlloc(src.length);
  EPASSERT_THROW(pBuffer, Result::AllocFailure, "Failed to allocate memory for UDConstantBuffer Cache");
  memcpy(pBuffer, src.ptr, src.length);
}

RenderConstantBuffer::~RenderConstantBuffer()
{
  epFree(pBuffer);
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
  EPTHROW_IF_NULL(pProgram, Result::Failure, "Failed to compile shader program");
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
static Variant getProgramData(epShaderProgram *pProgram, size_t location, size_t size)
{
  if (size > 1)
  {
    Array<T, 16> data(Alloc, size);
    epShader_GetProgramData(pProgram, location, data.getBuffer().ptr);
    return data;
  }
  else
  {
    T data;
    epShader_GetProgramData(pProgram, location, &data);
    return data;
  }
}

Variant RenderShaderProgram::getUniform(size_t i)
{
  epShaderElement t = epShader_GetUniformType(pProgram, i);
  EPASSERT(t.type >= epSET_Int && t.type <= epSET_Double && t.samplerType != epSST_None ? t.m == 1 && t.n == 1 && t.type == epSET_Float : true, "Invalid type");

  if (t.m == 1)
  {
    switch (t.n)
    {
      case 1:
        switch (t.type)
        {
          case epSET_Int:
            return getProgramData<int>(pProgram, t.location, t.arrayLength);
          case epSET_Uint:
            return getProgramData<uint32_t>(pProgram, t.location, t.arrayLength);
          case epSET_Float:
            return t.samplerType != epSST_None ? nullptr : getProgramData<float>(pProgram, t.location, t.arrayLength);
          case epSET_Double:
            return getProgramData<double>(pProgram, t.location, t.arrayLength);
        }
        break;
      case 2:
        switch (t.type)
        {
          case epSET_Int:
            return getProgramData<Vector2<int>>(pProgram, t.location, t.arrayLength);
          case epSET_Uint:
            return getProgramData<Vector2<uint32_t>>(pProgram, t.location, t.arrayLength);
          case epSET_Float:
            return getProgramData<Vector2<float>>(pProgram, t.location, t.arrayLength);
          case epSET_Double:
            return getProgramData<Vector2<double>>(pProgram, t.location, t.arrayLength);
        }
        break;
      case 3:
        switch (t.type)
        {
          case epSET_Int:
            return getProgramData<Vector3<int>>(pProgram, t.location, t.arrayLength);
          case epSET_Uint:
            return getProgramData<Vector3<uint32_t>>(pProgram, t.location, t.arrayLength);
          case epSET_Float:
            return getProgramData<Vector3<float>>(pProgram, t.location, t.arrayLength);
          case epSET_Double:
            return getProgramData<Vector3<double>>(pProgram, t.location, t.arrayLength);
        }
        break;
      case 4:
        switch (t.type)
        {
          case epSET_Int:
            return getProgramData<Vector4<int>>(pProgram, t.location, t.arrayLength);
          case epSET_Uint:
            return getProgramData<Vector4<uint32_t>>(pProgram, t.location, t.arrayLength);
          case epSET_Float:
            return getProgramData<Vector4<float>>(pProgram, t.location, t.arrayLength);
          case epSET_Double:
            return getProgramData<Vector4<double>>(pProgram, t.location, t.arrayLength);
        }
      default:
        EPTHROW(Result::Failure, "vector length {0} not supported", t.n);
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
          EPTHROW(Result::Failure, "Integer Matrix types not supported.");
        case epSET_Float:
          return getProgramData<Matrix4x4<float>>(pProgram, t.location, t.arrayLength);
        case epSET_Double:
          return getProgramData<Matrix4x4<double>>(pProgram, t.location, t.arrayLength);
      }
    }
    else
    {
      EPTHROW(Result::Failure, "Matrix {0},{1} not supported", t.m, t.n);
    }
  }
  else
  {
    EPTHROW(Result::Failure, "Unsupported element type m {0}, n {1}, type {2}", t.m, t.n, t.type);
  }

  return Variant();
}

void RenderShaderProgram::Use()
{
  epShader_SetCurrent(pProgram);
}

template <typename T>
static void setProgramData(size_t location, size_t size, const Variant &v)
{
  if (size > 1)
  {
    EPTHROW_IF(size > v.arrayLen(), Result::OutOfBounds, "Uniform array has insufficient length!");
    Array<T, 16> arr(Reserve, size);
    for (size_t j = 0; j < size; ++j)
      arr.push_back(v[j].as<T>());
    epShader_SetProgramData(location, arr.ptr, size);
  }
  else
    epShader_SetProgramData(location, v.as<T>());
}

void RenderShaderProgram::setTexture(size_t textureIndex, size_t uniformIndex, RenderTexture *pTexture)
{
  epShaderElement t = epShader_GetUniformType(pProgram, uniformIndex);
  epShader_SetProgramData(textureIndex, t.location, pTexture->pTexture);
}

void RenderShaderProgram::setUniform(size_t i, Variant v)
{
  epShaderElement t = epShader_GetUniformType(pProgram, i);
  epShaderElementType et = (epShaderElementType)t.type;
  EPASSERT(et >= epSET_Int && et <= epSET_Double && t.samplerType != epSST_None ? t.m == 1 && t.n == 1 && t.type == epSET_Float : true, "Invalid type");

  if (t.samplerType != epSST_None)
    return;

  if (t.m == 1)
  {
    switch (t.n)
    {
      case 1:
      {
        switch (et)
        {
          case epSET_Int:
            setProgramData<int>(t.location, t.arrayLength, v);
            return;
          case epSET_Uint:
            setProgramData<uint32_t>(t.location, t.arrayLength, v);
            return;
          case epSET_Float:
            setProgramData<float>(t.location, t.arrayLength, v);
            return;
          case epSET_Double:
            setProgramData<double>(t.location, t.arrayLength, v);
            return;
        }
      }
      case 2:
      {
        switch (et)
        {
          case epSET_Int:
            setProgramData<Vector2<int>>(t.location, t.arrayLength, v);
            return;
          case epSET_Uint:
            setProgramData<Vector2<uint32_t>>(t.location, t.arrayLength, v);
            return;
          case epSET_Float:
            setProgramData<Vector2<float>>(t.location, t.arrayLength, v);
            return;
          case epSET_Double:
            setProgramData<Vector2<double>>(t.location, t.arrayLength, v);
            return;
        }
      }
      case 3:
      {
        switch (t.type)
        {
          case epSET_Int:
            setProgramData<Vector3<int>>(t.location, t.arrayLength, v);
            return;
          case epSET_Uint:
            setProgramData<Vector3<uint32_t>>(t.location, t.arrayLength, v);
            return;
          case epSET_Float:
            setProgramData<Vector3<float>>(t.location, t.arrayLength, v);
            return;
          case epSET_Double:
            setProgramData<Vector3<double>>(t.location, t.arrayLength, v);
            return;
        }
      }
      case 4:
      {
        switch (t.type)
        {
          case epSET_Int:
            setProgramData<Vector4<int>>(t.location, t.arrayLength, v);
            return;
          case epSET_Uint:
            setProgramData<Vector4<uint32_t>>(t.location, t.arrayLength, v);
            return;
          case epSET_Float:
            setProgramData<Vector4<float>>(t.location, t.arrayLength, v);
            return;
          case epSET_Double:
            setProgramData<Vector4<double>>(t.location, t.arrayLength, v);
            return;
        }
      }
      default:
        EPTHROW(Result::Failure, "vector length {0} not supported", t.n);
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
          EPTHROW(Result::Failure, "Integer Matrix types not supported.");
          return;
        case epSET_Float:
          setProgramData<Matrix4x4<float>>(t.location, t.arrayLength, v);
          return;
        case epSET_Double:
          setProgramData<Matrix4x4<double>>(t.location, t.arrayLength, v);
          return;
      }
    }
    else
    {
      EPTHROW(Result::Failure, "Matrix {0},{1} not supported", t.m, t.n);
    }
  }
  else
  {
    EPTHROW(Result::Failure, "Unsupported element type m {0}, n {1}, type {2}", t.m, t.n, t.type);
  }
}

RenderShaderInputConfig::RenderShaderInputConfig(Renderer *pRenderer, SharedArray<ArrayBufferRef> vertexArrays, RenderShaderProgramRef spProgram)
  : RenderResource(pRenderer)
{
  Array<epArrayElement, 8> elementArray;
  for (size_t i = 0; i < vertexArrays.length; ++i)
  {
    Variant varAttribs = vertexArrays[i]->getMetadata()->get("attributeInfo");
    EPASSERT_THROW(varAttribs.isValid(), Result::Failure, "attribute info not present in buffer metadata");
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
