#pragma once
#ifndef UDRENDERRESOURCE_H
#define UDRENDERRESOURCE_H

#include "components/resources/array.h"
#include "components/resources/model.h"
#include "hal/shader.h"

struct udFormatDeclaration;
struct udArrayElement;
struct udArrayBuffer;
struct udTexture;
struct udShader;
struct udShaderProgram;

namespace ud
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
class RenderResource : public udRefCounted
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

  udArrayBuffer *pArray;
};
SHARED_CLASS(RenderArray);

// render texture
class RenderTexture : public RenderResource
{
public:
  RenderTexture(Renderer *pRenderer, ArrayBufferRef spArrayBuffer, TextureUsage usage);
  virtual ~RenderTexture();

  udTexture *pTexture;
};
SHARED_CLASS(RenderTexture);

// render shader
class RenderShader : public RenderResource
{
public:
  RenderShader(Renderer *pRenderer, ShaderRef spShader, udShaderType type);
  virtual ~RenderShader();

  udShader *pShader;
  udShaderType type;
};
SHARED_CLASS(RenderShader);

// render shader program
class RenderShaderProgram : public RenderResource
{
public:
  RenderShaderProgram(Renderer *pRenderer, RenderShaderRef vs, RenderShaderRef ps);
  virtual ~RenderShaderProgram();

  udShaderProgram *pProgram;
};
SHARED_CLASS(RenderShaderProgram);

// render format descriptor
class RenderVertexFormat : public RenderResource
{
public:
  RenderVertexFormat(Renderer *pRenderer, const udArrayElement *pElements, size_t numElements);
  virtual ~RenderVertexFormat();

  udFormatDeclaration *pFormat;
};
SHARED_CLASS(RenderVertexFormat);

} // namespace ud

#endif // UDRENDERRESOURCE_H
