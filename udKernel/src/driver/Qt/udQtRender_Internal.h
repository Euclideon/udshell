#pragma once
#ifndef UDQTRENDER_INTERNAL_H
#define UDQTRENDER_INTERNAL_H

#include "hal/driver.h"
#include "hal/texture.h"
#include "hal/vertex.h"

#include <QOpenGLFunctions_2_0>
#include <QOpenGLDebugLogger>

// internal storage of the Qt GL Context
struct udQtGLContext
{
  QOpenGLFunctions_2_0 *pFunc;
  QOpenGLDebugLogger *pDebugger;
};

extern udQtGLContext s_QtGLContext;


class QOpenGLTexture;
class QOpenGLShader;
class QOpenGLShaderProgram;
class QOpenGLBuffer;

struct udTexture
{
  udTextureType type;
  udImageFormat format;
  size_t width, height, depth;
  size_t elements;
  int levels;
  QOpenGLTexture *pTexture;
};

struct udArrayElementData
{
  int offset;
  int stride;
};

struct udFormatDeclaration
{
  udArrayElement *pElements;
  udArrayElementData *pElementData;
  int numElements;
};

struct udArrayBuffer
{
  QOpenGLBuffer *pBuffer;

  udArrayDataFormat *pFormat;
  size_t numElements;
};

struct udShader
{
  QOpenGLShader *pShader;
};

struct udShaderProgram
{
  QOpenGLShaderProgram *pProgram;

  struct Param
  {
    char *pName;
    size_t type;
  };
  size_t numAttributes;
  Param *pAttributes;
  size_t numUniforms;
  Param *pUniforms;
};

#endif  // UDQTRENDER_INTERNAL_H
