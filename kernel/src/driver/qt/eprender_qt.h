#pragma once
#ifndef eprender_qt_H
#define eprender_qt_H

#include "hal/driver.h"
#include "hal/texture.h"
#include "hal/vertex.h"

#include "driver/qt/epqt.h"

#include <QOpenGLFunctions>
#include <QOpenGLFunctions_3_2_Core>
#include <QOpenGLDebugLogger>

// internal storage of the Qt GL Context
struct epQtGLContext
{
  QOpenGLFunctions *pFunc;
  QOpenGLFunctions_3_2_Core *pFunc3_2_Core;
};

extern epQtGLContext s_QtGLContext;


class QOpenGLTexture;
class QOpenGLShader;
class QOpenGLShaderProgram;
class QOpenGLBuffer;

struct epTexture
{
  epTextureType type;
  epImageFormat format;
  size_t width, height, depth;
  size_t elements;
  int levels;
  QOpenGLTexture *pTexture;
};

struct epArrayElementData
{
  int offset;
  int stride;
};

struct epFormatDeclaration
{
  epArrayElement *pElements;
  epArrayElementData *pElementData;
  int numElements;
};

struct epArrayBuffer
{
  QOpenGLBuffer *pBuffer;

  epArrayDataFormat *pFormat;
  size_t numElements;
};

struct epShader
{
  QOpenGLShader *pShader;
};

struct epShaderProgram
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

#endif  // eprender_qt_H
