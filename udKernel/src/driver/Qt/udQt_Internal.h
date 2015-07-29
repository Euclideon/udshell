#pragma once
#ifndef UDQT_INTERNAL_H
#define UDQT_INTERNAL_H

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

struct udVertexElementData
{
  int offset;
  int stride;
};

struct udVertexDeclaration
{
  udVertexElement *pElements;
  udVertexElementData *pElementData;
  int numElements;
};

struct udVertexBuffer
{
  udVertexDeclaration *pVertexDeclaration;

  QOpenGLBuffer *pVB;
};

struct udShader
{
  QOpenGLShader *pShader;
};

struct udShaderProgram
{
  QOpenGLShaderProgram *pProgram;
};

#endif  // UDQT_INTERNAL_H
