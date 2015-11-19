#pragma once
#ifndef EPRENDERRESOURCE_H
#define EPRENDERRESOURCE_H

#include "components/resources/array.h"
#include "components/resources/model.h"
#include "hal/shader.h"

struct epFormatDeclaration;
struct epArrayElement;
struct epArrayBuffer;
struct epTexture;
struct epShader;
struct epShaderProgram;

namespace kernel
{

class Renderer;

enum class ArrayUsage
{
  VertexData,
  IndexData
};

enum class TextureUsage
{
  _1D,
  _1DArray,
  _2D,
  _2DArray,
  _Cube,
  _CubeArray,
  _3D,
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
  RenderTexture(Renderer *pRenderer, ArrayBufferRef spArrayBuffer, TextureUsage usage);
  virtual ~RenderTexture();

  epTexture *pTexture;
};
SHARED_CLASS(RenderTexture);

// render shader
class RenderShader : public RenderResource
{
public:
  RenderShader(Renderer *pRenderer, ShaderRef spShader, epShaderType type);
  virtual ~RenderShader();

  epShader *pShader;
  epShaderType type;
};
SHARED_CLASS(RenderShader);

// render shader program
class RenderShaderProgram : public RenderResource
{
public:
  RenderShaderProgram(Renderer *pRenderer, RenderShaderRef vs, RenderShaderRef ps);
  virtual ~RenderShaderProgram();

  epShaderProgram *pProgram;

  size_t numAttributes();
  String getAttributeName(size_t i);

  size_t numUniforms();
  String getUniformName(size_t i);

  void setUniform(int i, const Float4 &v4);
};
SHARED_CLASS(RenderShaderProgram);

// render format descriptor
class RenderVertexFormat : public RenderResource
{
public:
  RenderVertexFormat(Renderer *pRenderer, const epArrayElement *pElements, size_t numElements);
  virtual ~RenderVertexFormat();

  epFormatDeclaration *pFormat;
};
SHARED_CLASS(RenderVertexFormat);

} // namespace kernel

#endif // EPRENDERRESOURCE_H
