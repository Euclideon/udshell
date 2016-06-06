#pragma once
#ifndef EPRENDERRESOURCE_H
#define EPRENDERRESOURCE_H
#include "ep/cpp/platform.h"
#include "ep/cpp/component/resource/arraybuffer.h"
#include "ep/cpp/component/resource/model.h"
#include "ep/cpp/component/resource/shader.h"
#include "hal/shader.h"
#include "hal/texture.h"

struct epShaderInputConfig;
struct epArrayElement;
struct epArrayBuffer;
struct epTexture;
struct epShader;
struct epShaderProgram;

namespace ep {

class Renderer;

enum class ArrayUsage
{
  VertexData,
  IndexData
};

// base render resource
class RenderResource : public RefCounted
{
public:
  RenderResource(Renderer *pRenderer) : pRenderer(pRenderer) {}
  virtual ~RenderResource() = 0;

  Renderer *pRenderer;
};
inline RenderResource::~RenderResource() {}
SHARED_CLASS(RenderResource);

// render array buffer
class RenderArray : public RenderResource
{
public:
  RenderArray(Renderer *pRenderer, ArrayBufferRef spArrayBuffer, ArrayUsage usage);
  virtual ~RenderArray();

  epArrayBuffer *pArray;
};
SHARED_CLASS(RenderArray);

// render texture
class RenderTexture : public RenderResource
{
public:
  RenderTexture(Renderer *pRenderer, ArrayBufferRef spArrayBuffer);
  virtual ~RenderTexture();

  epTexture *pTexture;
  epTextureType usage;
  epImageFormat format;
};
SHARED_CLASS(RenderTexture);

class RenderConstantBuffer : public RenderResource
{
public:
  RenderConstantBuffer(Renderer *pRenderer, BufferRef spBuffer);
  virtual ~RenderConstantBuffer();

  void *pBuffer;
};
SHARED_CLASS(RenderConstantBuffer);

// render shader
class RenderShader : public RenderResource
{
public:
  RenderShader(Renderer *pRenderer, SharedString code, epShaderType type);
  virtual ~RenderShader();

  epShader *pShader;
  epShaderType type;
};
SHARED_CLASS(RenderShader);

// render shader program
class RenderShaderProgram : public RenderResource
{
public:
  RenderShaderProgram(Renderer *pRenderer, Slice<RenderShaderRef> shaders);
  virtual ~RenderShaderProgram();

  size_t numAttributes();
  String getAttributeName(size_t i);
  epShaderElement getAttributeType(size_t i);
  SharedString getAttributeTypeString(size_t i);

  size_t numUniforms();
  String getUniformName(size_t i);
  epShaderElement getUniformType(size_t i);
  SharedString getUniformTypeString(size_t i);

  Variant getUniform(size_t i);

  void Use();
  void setUniform(size_t i, Variant v);

  epShaderProgram *pProgram;
  bool uniformsAssigned;
};
SHARED_CLASS(RenderShaderProgram);

// render shader input
class RenderShaderInputConfig : public RenderResource
{
public:
  RenderShaderInputConfig(Renderer *pRenderer, SharedArray<ArrayBufferRef> vertexArrays, RenderShaderProgramRef spProgram);
  SharedArray<int> GetActiveStreams();
  virtual ~RenderShaderInputConfig();

  epShaderInputConfig *pConfig;
};
SHARED_CLASS(RenderShaderInputConfig);

int GetElementTypeSize(String type);

} // namespace ep

#endif // EPRENDERRESOURCE_H
