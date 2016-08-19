#include "driver.h"

#if EPRENDER_DRIVER == EPDRIVER_QT

#include "hal/shader.h"
#include "hal/vertex.h"

#include "eprender_qt.h"

#include "ep/cpp/error.h"

#include <QOpenGLShaderProgram>
#include <QOpenGLShader>
#include <QOpenGLTexture>
#include <QOpenGLFunctions>

static QOpenGLShader::ShaderTypeBit s_shaderType[epST_Max] =
{
  QOpenGLShader::Vertex,
  QOpenGLShader::Fragment,
  QOpenGLShader::Geometry,
  QOpenGLShader::TessellationControl,
  QOpenGLShader::TessellationEvaluation,
  QOpenGLShader::Compute,
};


// ***************************************************************************************
epShader* epShader_CreateShaderFromFile(const char *epUnusedParam(pFilename), epShaderType epUnusedParam(type))
{
  EPASSERT(false, "TODO");
  return 0;
}

// ***************************************************************************************
epShader* epShader_CreateShader(const char *pSource, size_t length, epShaderType type)
{
  // HACK HACK
  if (s_QtGLContext.singleThreadMode)
  {
    if (!QOpenGLContext::currentContext())
      s_QtGLContext.pContext->makeCurrent(s_QtGLContext.pSurface);
  }

  QOpenGLShader *pQtShader = new QOpenGLShader(s_shaderType[type]);
  if (!pQtShader->compileSourceCode(QByteArray(pSource, static_cast<int>(length))))
  {
    epDebugPrintf("Shader failed to compile: %s\n", pQtShader->log().toUtf8().data());
    delete pQtShader;
    return 0;
  }

  epShader *pShader = epAllocType(epShader, 1, epAF_None);
  pShader->pShader = pQtShader;

  return pShader;
}

// ***************************************************************************************
// Author: David Ely, February 2016
void epShader_DestroyShader(epShader **ppShader)
{
  // HACK HACK
  if (s_QtGLContext.singleThreadMode)
  {
    if (!QOpenGLContext::currentContext())
      s_QtGLContext.pContext->makeCurrent(s_QtGLContext.pSurface);
  }

  EPASSERT(ppShader && *ppShader, "ppShader is invalid");
  epShader *pShader = *ppShader;
  delete pShader->pShader;
  epFree(pShader);
  *ppShader = nullptr;
}

// ***************************************************************************************
static epShaderElement ConvertToShaderElementType(size_t type, uint32_t length, uint32_t location)
{
  epShaderElement uniformType;
  (uint64_t&)uniformType = 0;
  uniformType.arrayLength = length;
  uniformType.location = location;

  switch (type)
  {
    case GL_FLOAT:
    {
      uniformType.m = 1;
      uniformType.n = 1;
      uniformType.type = epSET_Float;
      break;
    }
    case GL_FLOAT_VEC2:
    {
      uniformType.m = 1;
      uniformType.n = 2;
      uniformType.type = epSET_Float;
      break;
    }
    case GL_FLOAT_VEC3:
    {
      uniformType.m = 1;
      uniformType.n = 3;
      uniformType.type = epSET_Float;
      break;
    }
    case GL_FLOAT_VEC4:
    {
      uniformType.m = 1;
      uniformType.n = 4;
      uniformType.type = epSET_Float;
      break;
    }
    case GL_FLOAT_MAT2:
    {
      uniformType.m = 2;
      uniformType.n = 2;
      uniformType.type = epSET_Float;
      break;
    }
    case GL_FLOAT_MAT3:
    {
      uniformType.m = 3;
      uniformType.n = 3;
      uniformType.type = epSET_Float;
      break;
    }
    case GL_FLOAT_MAT4:
    {
      uniformType.m = 4;
      uniformType.n = 4;
      uniformType.type = epSET_Float;
      break;
    }
    case GL_FLOAT_MAT2x3:
    {
      uniformType.m = 2;
      uniformType.n = 3;
      uniformType.type = epSET_Float;
      break;
    }
    case GL_FLOAT_MAT2x4:
    {
      uniformType.m = 2;
      uniformType.n = 4;
      uniformType.type = epSET_Float;
      break;
    }
    case GL_FLOAT_MAT3x2:
    {
      uniformType.m = 3;
      uniformType.n = 2;
      uniformType.type = epSET_Float;
      break;
    }
    case GL_FLOAT_MAT3x4:
    {
      uniformType.m = 3;
      uniformType.n = 4;
      uniformType.type = epSET_Float;
      break;
    }
    case GL_FLOAT_MAT4x2:
    {
      uniformType.m = 4;
      uniformType.n = 2;
      uniformType.type = epSET_Float;
      break;
    }
    case GL_FLOAT_MAT4x3:
    {
      uniformType.m = 4;
      uniformType.n = 3;
      uniformType.type = epSET_Float;
      break;
    }
    case GL_INT:
    {
      uniformType.m = 1;
      uniformType.n = 1;
      uniformType.type = epSET_Int;
      break;
    }
    case GL_INT_VEC2:
    {
      uniformType.m = 1;
      uniformType.n = 2;
      uniformType.type = epSET_Int;
      break;
    }
    case GL_INT_VEC3:
    {
      uniformType.m = 1;
      uniformType.n = 3;
      uniformType.type = epSET_Int;
      break;
    }
    case GL_INT_VEC4:
    {
      uniformType.m = 1;
      uniformType.n = 4;
      uniformType.type = epSET_Int;
      break;
    }
    case GL_UNSIGNED_INT:
    {
      uniformType.m = 1;
      uniformType.n = 1;
      uniformType.type = epSET_Uint;
      break;
    }
    case GL_UNSIGNED_INT_VEC2:
    {
      uniformType.m = 1;
      uniformType.n = 2;
      uniformType.type = epSET_Uint;
      break;
    }
    case GL_UNSIGNED_INT_VEC3:
    {
      uniformType.m = 1;
      uniformType.n = 3;
      uniformType.type = epSET_Uint;
      break;
    }
    case GL_UNSIGNED_INT_VEC4:
    {
      uniformType.m = 1;
      uniformType.n = 4;
      uniformType.type = epSET_Uint;
      break;
    }
    case GL_DOUBLE:
    {
      uniformType.m = 1;
      uniformType.n = 1;
      uniformType.type = epSET_Double;
      break;
    }
    case GL_DOUBLE_VEC2:
    {
      uniformType.m = 1;
      uniformType.n = 2;
      uniformType.type = epSET_Double;
      break;
    }
    case GL_DOUBLE_VEC3:
    {
      uniformType.m = 1;
      uniformType.n = 3;
      uniformType.type = epSET_Double;
      break;
    }
    case GL_DOUBLE_VEC4:
    {
      uniformType.m = 1;
      uniformType.n = 4;
      uniformType.type = epSET_Double;
      break;
    }
    case GL_DOUBLE_MAT2:
    {
      uniformType.m = 2;
      uniformType.n = 2;
      uniformType.type = epSET_Double;
      break;
    }
    case GL_DOUBLE_MAT3:
    {
      uniformType.m = 3;
      uniformType.n = 3;
      uniformType.type = epSET_Double;
      break;
    }
    case GL_DOUBLE_MAT4:
    {
      uniformType.m = 4;
      uniformType.n = 4;
      uniformType.type = epSET_Double;
      break;
    }
    case GL_DOUBLE_MAT2x3:
    {
      uniformType.m = 2;
      uniformType.n = 3;
      uniformType.type = epSET_Double;
      break;
    }
    case GL_DOUBLE_MAT2x4:
    {
      uniformType.m = 2;
      uniformType.n = 4;
      uniformType.type = epSET_Double;
      break;
    }
    case GL_DOUBLE_MAT3x2:
    {
      uniformType.m = 3;
      uniformType.n = 2;
      uniformType.type = epSET_Double;
      break;
    }
    case GL_DOUBLE_MAT3x4:
    {
      uniformType.m = 3;
      uniformType.n = 4;
      uniformType.type = epSET_Double;
      break;
    }
    case GL_DOUBLE_MAT4x2:
    {
      uniformType.m = 4;
      uniformType.n = 2;
      uniformType.type = epSET_Double;
      break;
    }
    case GL_DOUBLE_MAT4x3:
    {
      uniformType.m = 4;
      uniformType.n = 3;
      uniformType.type = epSET_Double;
      break;
    }
    case GL_SAMPLER_1D:
    case GL_SAMPLER_1D_SHADOW:
    case GL_SAMPLER_1D_ARRAY:
    case GL_SAMPLER_1D_ARRAY_SHADOW:
    case GL_SAMPLER_2D:
    case GL_SAMPLER_2D_RECT:
    case GL_SAMPLER_2D_SHADOW:
    case GL_SAMPLER_2D_RECT_SHADOW:
    case GL_SAMPLER_2D_ARRAY:
    case GL_SAMPLER_2D_ARRAY_SHADOW:
    case GL_SAMPLER_3D:
    case GL_SAMPLER_CUBE:
    case GL_SAMPLER_CUBE_SHADOW:
    case GL_SAMPLER_CUBE_MAP_ARRAY:
    case GL_SAMPLER_CUBE_MAP_ARRAY_SHADOW:
    case GL_SAMPLER_BUFFER:
    case GL_INT_SAMPLER_1D:
    case GL_INT_SAMPLER_1D_ARRAY:
    case GL_INT_SAMPLER_2D:
    case GL_INT_SAMPLER_2D_MULTISAMPLE:
    case GL_INT_SAMPLER_2D_ARRAY:
    case GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
    case GL_INT_SAMPLER_2D_RECT:
    case GL_INT_SAMPLER_3D:
    case GL_INT_SAMPLER_CUBE:
    case GL_INT_SAMPLER_CUBE_MAP_ARRAY:
    case GL_INT_SAMPLER_BUFFER:
    case GL_UNSIGNED_INT_SAMPLER_1D:
    case GL_UNSIGNED_INT_SAMPLER_1D_ARRAY:
    case GL_UNSIGNED_INT_SAMPLER_2D:
    case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE:
    case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY:
    case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
    case GL_UNSIGNED_INT_SAMPLER_2D_RECT:
    case GL_UNSIGNED_INT_SAMPLER_3D:
    case GL_UNSIGNED_INT_SAMPLER_CUBE:
    case GL_UNSIGNED_INT_SAMPLER_CUBE_MAP_ARRAY:
    case GL_UNSIGNED_INT_SAMPLER_BUFFER:
    {
      uniformType.m = 1;
      uniformType.n = 1;
      switch (type)
      {
        case GL_SAMPLER_1D:
        case GL_SAMPLER_1D_SHADOW:
        case GL_SAMPLER_1D_ARRAY:
        case GL_SAMPLER_1D_ARRAY_SHADOW:
        case GL_SAMPLER_2D:
        case GL_SAMPLER_2D_RECT:
        case GL_SAMPLER_2D_SHADOW:
        case GL_SAMPLER_2D_RECT_SHADOW:
        case GL_SAMPLER_2D_ARRAY:
        case GL_SAMPLER_2D_ARRAY_SHADOW:
        case GL_SAMPLER_3D:
        case GL_SAMPLER_CUBE:
        case GL_SAMPLER_CUBE_SHADOW:
        case GL_SAMPLER_CUBE_MAP_ARRAY:
        case GL_SAMPLER_CUBE_MAP_ARRAY_SHADOW:
        case GL_SAMPLER_BUFFER:
          uniformType.type = epSET_Float; break;
        case GL_INT_SAMPLER_1D:
        case GL_INT_SAMPLER_1D_ARRAY:
        case GL_INT_SAMPLER_2D:
        case GL_INT_SAMPLER_2D_MULTISAMPLE:
        case GL_INT_SAMPLER_2D_ARRAY:
        case GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
        case GL_INT_SAMPLER_2D_RECT:
        case GL_INT_SAMPLER_3D:
        case GL_INT_SAMPLER_CUBE:
        case GL_INT_SAMPLER_CUBE_MAP_ARRAY:
        case GL_INT_SAMPLER_BUFFER:
          uniformType.type = epSET_Int; break;
        case GL_UNSIGNED_INT_SAMPLER_1D:
        case GL_UNSIGNED_INT_SAMPLER_1D_ARRAY:
        case GL_UNSIGNED_INT_SAMPLER_2D:
        case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE:
        case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY:
        case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
        case GL_UNSIGNED_INT_SAMPLER_2D_RECT:
        case GL_UNSIGNED_INT_SAMPLER_3D:
        case GL_UNSIGNED_INT_SAMPLER_CUBE:
        case GL_UNSIGNED_INT_SAMPLER_CUBE_MAP_ARRAY:
        case GL_UNSIGNED_INT_SAMPLER_BUFFER:
          uniformType.type = epSET_Uint; break;
      }
      switch (type)
      {
        case GL_SAMPLER_1D:
        case GL_SAMPLER_1D_ARRAY:
        case GL_SAMPLER_2D:
        case GL_SAMPLER_2D_ARRAY:
        case GL_SAMPLER_3D:
        case GL_SAMPLER_CUBE:
        case GL_SAMPLER_CUBE_MAP_ARRAY:
        case GL_INT_SAMPLER_1D:
        case GL_INT_SAMPLER_1D_ARRAY:
        case GL_INT_SAMPLER_2D:
        case GL_INT_SAMPLER_2D_ARRAY:
        case GL_INT_SAMPLER_3D:
        case GL_INT_SAMPLER_CUBE:
        case GL_INT_SAMPLER_CUBE_MAP_ARRAY:
        case GL_UNSIGNED_INT_SAMPLER_1D:
        case GL_UNSIGNED_INT_SAMPLER_1D_ARRAY:
        case GL_UNSIGNED_INT_SAMPLER_2D:
        case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY:
        case GL_UNSIGNED_INT_SAMPLER_3D:
        case GL_UNSIGNED_INT_SAMPLER_CUBE:
        case GL_UNSIGNED_INT_SAMPLER_CUBE_MAP_ARRAY:
          uniformType.samplerType = epSST_Default; break;
        case GL_SAMPLER_2D_RECT:
        case GL_INT_SAMPLER_2D_RECT:
        case GL_UNSIGNED_INT_SAMPLER_2D_RECT:
          uniformType.samplerType = epSST_Rect; break;
        case GL_SAMPLER_1D_SHADOW:
        case GL_SAMPLER_1D_ARRAY_SHADOW:
        case GL_SAMPLER_2D_SHADOW:
        case GL_SAMPLER_2D_ARRAY_SHADOW:
        case GL_SAMPLER_CUBE_SHADOW:
        case GL_SAMPLER_CUBE_MAP_ARRAY_SHADOW:
          uniformType.samplerType = epSST_Shadow; break;
        case GL_SAMPLER_2D_RECT_SHADOW:
          uniformType.samplerType = epSST_ShadowRect; break;
        case GL_INT_SAMPLER_2D_MULTISAMPLE:
        case GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
        case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE:
        case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
          uniformType.samplerType = epSST_Multisample; break;
        case GL_SAMPLER_BUFFER:
        case GL_INT_SAMPLER_BUFFER:
        case GL_UNSIGNED_INT_SAMPLER_BUFFER:
          uniformType.samplerType = epSST_Buffer; break;
      }
      switch (type)
      {
        case GL_SAMPLER_1D:
        case GL_SAMPLER_1D_SHADOW:
        case GL_INT_SAMPLER_1D:
        case GL_UNSIGNED_INT_SAMPLER_1D:
          uniformType.samplerDimensions = epSSD_1D; break;
        case GL_SAMPLER_1D_ARRAY:
        case GL_SAMPLER_1D_ARRAY_SHADOW:
        case GL_INT_SAMPLER_1D_ARRAY:
        case GL_UNSIGNED_INT_SAMPLER_1D_ARRAY:
          uniformType.samplerDimensions = epSSD_1DArray; break;
        case GL_SAMPLER_2D:
        case GL_SAMPLER_2D_RECT:
        case GL_SAMPLER_2D_SHADOW:
        case GL_SAMPLER_2D_RECT_SHADOW:
        case GL_INT_SAMPLER_2D:
        case GL_INT_SAMPLER_2D_MULTISAMPLE:
        case GL_INT_SAMPLER_2D_RECT:
        case GL_UNSIGNED_INT_SAMPLER_2D:
        case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE:
        case GL_UNSIGNED_INT_SAMPLER_2D_RECT:
          uniformType.samplerDimensions = epSSD_2D; break;
        case GL_SAMPLER_2D_ARRAY:
        case GL_SAMPLER_2D_ARRAY_SHADOW:
        case GL_INT_SAMPLER_2D_ARRAY:
        case GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
        case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY:
        case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
          uniformType.samplerDimensions = epSSD_2DArray; break;
        case GL_SAMPLER_3D:
        case GL_INT_SAMPLER_3D:
        case GL_UNSIGNED_INT_SAMPLER_3D:
          uniformType.samplerDimensions = epSSD_3D; break;
        case GL_SAMPLER_CUBE:
        case GL_SAMPLER_CUBE_SHADOW:
        case GL_INT_SAMPLER_CUBE:
        case GL_UNSIGNED_INT_SAMPLER_CUBE:
          uniformType.samplerDimensions = epSSD_Cube; break;
        case GL_SAMPLER_CUBE_MAP_ARRAY:
        case GL_SAMPLER_CUBE_MAP_ARRAY_SHADOW:
        case GL_INT_SAMPLER_CUBE_MAP_ARRAY:
        case GL_UNSIGNED_INT_SAMPLER_CUBE_MAP_ARRAY:
          uniformType.samplerDimensions = epSSD_CubeArray; break;
        case GL_SAMPLER_BUFFER:
        case GL_INT_SAMPLER_BUFFER:
        case GL_UNSIGNED_INT_SAMPLER_BUFFER:
          uniformType.samplerDimensions = epSSD_Buffer; break;
      }
    }
  }

  return uniformType;
}

// ***************************************************************************************
epShaderProgram* epShader_CreateShaderProgram(epShader *shaders[], size_t arrayLength)
{
  // HACK HACK
  if (s_QtGLContext.singleThreadMode)
  {
    if (!QOpenGLContext::currentContext())
      s_QtGLContext.pContext->makeCurrent(s_QtGLContext.pSurface);
  }

  bool result = true;
  epShaderProgram *pProgram = nullptr;

  QOpenGLShaderProgram *pQtProgram = new QOpenGLShaderProgram();
  for (size_t i = 0; i < arrayLength; ++i)
    EP_ERROR_IF(!pQtProgram->addShader(shaders[i]->pShader), false);
  EP_ERROR_IF(!pQtProgram->link(), false);

  pQtProgram->removeAllShaders();

  {
    pQtProgram->bind();
    QOpenGLFunctions funcs(QOpenGLContext::currentContext());
    GLuint program = pQtProgram->programId();

    size_t extraBytes = 0;

    GLint numAttributes;
    funcs.glGetProgramiv(program, GL_ACTIVE_ATTRIBUTES, &numAttributes);
    for (GLint i = 0; i < numAttributes; ++i)
    {
      char name[128];
      GLsizei length;
      GLint size;
      GLenum type;
      funcs.glGetActiveAttrib(program, i, sizeof(name), &length, &size, &type, name);
      extraBytes += length + 1;
    }

    GLint numUniforms;
    funcs.glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &numUniforms);
    for (GLint i = 0; i < numUniforms; ++i)
    {
      char name[128];
      GLsizei length;
      GLint size;
      GLenum type;
      funcs.glGetActiveUniform(program, i, sizeof(name), &length, &size, &type, name);
      extraBytes += length + 1;
    }

    extraBytes += sizeof(epShaderProgram::Param) * (numAttributes + numUniforms);

    pProgram = (epShaderProgram*)epAlloc(sizeof(epShaderProgram) + extraBytes);
    pProgram->pProgram = pQtProgram;
    pProgram->numAttributes = numAttributes;
    pProgram->numUniforms = numUniforms;
    pProgram->pAttributes = (epShaderProgram::Param*)&pProgram[1];
    pProgram->pUniforms = (epShaderProgram::Param*)&pProgram->pAttributes[numAttributes];
    char *pStrings = (char*)&pProgram->pUniforms[numUniforms];
    char *pEnd = (char*)&pProgram[1] + extraBytes;

    for (GLint i = 0; i < numAttributes; ++i)
    {
      GLsizei length;
      GLint size;
      GLenum type;
      pProgram->pAttributes[i].pName = pStrings;
      funcs.glGetActiveAttrib(program, i, pEnd - pStrings, &length, &size, &type, pProgram->pAttributes[i].pName);
      pProgram->pAttributes[i].elementType = ConvertToShaderElementType(type, size, pQtProgram->attributeLocation(pProgram->pAttributes[i].pName));
      pStrings += length + 1;
    }

    for (GLint i = 0; i < numUniforms; ++i)
    {
      GLsizei length;
      GLint size;
      GLenum type;
      pProgram->pUniforms[i].pName = pStrings;
      funcs.glGetActiveUniform(program, i, pEnd - pStrings, &length, &size, &type, pProgram->pUniforms[i].pName);
      if (size > 1)
      {
        // array uniforms seem to have "[0]" attached to the end of the uniform names!
        length -= 3;
        pProgram->pUniforms[i].pName[length] = '\0';
      }
      pProgram->pUniforms[i].elementType = ConvertToShaderElementType(type, size, pQtProgram->uniformLocation(pProgram->pUniforms[i].pName));
      pStrings += length + 1;
    }
  }

epilogue:
  if (!result)
  {
    epDebugPrintf("Error creating shader program: %s\n", pQtProgram->log().toUtf8().data());
    delete pQtProgram;
  }

  return pProgram;
}

// ***************************************************************************************
void epShader_DestroyShaderProgram(epShaderProgram **ppProgram)
{
  // HACK HACK
  if (s_QtGLContext.singleThreadMode)
  {
    if (!QOpenGLContext::currentContext())
      s_QtGLContext.pContext->makeCurrent(s_QtGLContext.pSurface);
  }

  EPASSERT(ppProgram && *ppProgram, "ppProgram is invalid");
  epShaderProgram *pProgram = *ppProgram;
  delete pProgram->pProgram;
  epFree(pProgram);
  *ppProgram = nullptr;
}

// ***************************************************************************************
size_t epShader_GetNumAttributes(epShaderProgram *pProgram)
{
  return pProgram->numAttributes;
}

// ***************************************************************************************
const char *epShader_GetAttributeName(epShaderProgram *pProgram, size_t i)
{
  return pProgram->pAttributes[i].pName;
}

// ***************************************************************************************
epShaderElement epShader_GetAttributeType(epShaderProgram *pProgram, size_t i)
{
  return pProgram->pAttributes[i].elementType;
}

// ***************************************************************************************
size_t epShader_GetNumUniforms(epShaderProgram *pProgram)
{
  return pProgram->numUniforms;
}

// ***************************************************************************************
const char *epShader_GetUniformName(epShaderProgram *pProgram, size_t i)
{
  return pProgram->pUniforms[i].pName;
}

// ***************************************************************************************
epShaderElement epShader_GetUniformType(epShaderProgram *pProgram, size_t i)
{
  return pProgram->pUniforms[i].elementType;
}

// ***************************************************************************************
void epShader_GetElementTypeString(epShaderElement t, char *pBuffer, size_t size, size_t *pLength)
{
  if (!pBuffer || size < 10)
  {
    EPASSERT(false, "Invalid Parameter(s)");
    return;
  }

  size_t length = 3;

  switch (t.type)
  {
    case epSET_Int:
    {
      memcpy(pBuffer, "s32", 3);
      break;
    }
    case epSET_Uint:
    {
      memcpy(pBuffer, "u32", 3);
      break;
    }
    case epSET_Float:
    {
      if (t.samplerType == epSST_Default)
      {
        length = 7;
        memcpy(pBuffer, "sampler", 7);
      }
      else
        memcpy(pBuffer, "f32", 3);
      break;
    }
    case epSET_Double:
    {
      memcpy(pBuffer, "f64", 3);
      break;
    }
    default:
      EPASSERT(0, "Invalid element type");
      break;
  }

  if (t.m > 1)
  {
    char *pArr = &pBuffer[length];

    pArr[0] = '[';
    pArr[1] = '0' + char(t.m);
    pArr[2] = ']';
    length += 3;
  }

  if (t.n > 1)
  {
    char *pArr = &pBuffer[length];

    pArr[0] = '[';
    pArr[1] = '0' + char(t.n);
    pArr[2] = ']';

    length += 3;
  }

  *pLength = length;
}

// ***************************************************************************************
void epShader_GetProgramData(epShaderProgram *pProgram, size_t param, bool *pValue)
{
  GLint glInt;
  s_QtGLContext.pFunc->glGetUniformiv(pProgram->pProgram->programId(), (int)param, &glInt);
  *pValue = (bool)glInt;
}

// ***************************************************************************************
void epShader_GetProgramData(epShaderProgram *pProgram, size_t param, int *pValue)
{
  s_QtGLContext.pFunc->glGetUniformiv(pProgram->pProgram->programId(), (int)param, pValue);
}

// ***************************************************************************************
void epShader_GetProgramData(epShaderProgram *pProgram, size_t param, uint32_t *pValue)
{
  s_QtGLContext.pFunc->glGetUniformiv(pProgram->pProgram->programId(), (int)param, (int*)pValue);
}

// ***************************************************************************************
void epShader_GetProgramData(epShaderProgram *pProgram, size_t param, float *pValue)
{
  GLfloat glFloat;
  s_QtGLContext.pFunc->glGetUniformfv(pProgram->pProgram->programId(), (int)param, &glFloat);

  *pValue = (float)glFloat;
}

// ***************************************************************************************
void epShader_GetProgramData(epShaderProgram *pProgram, size_t param, double *pValue)
{
  if (s_QtGLContext.pFunc4_0_Core)
    s_QtGLContext.pFunc4_0_Core->glGetUniformdv(pProgram->pProgram->programId(), (int)param, pValue);
}

// ***************************************************************************************
void epShader_GetProgramData(epShaderProgram *pProgram, size_t param, ep::Vector2<int> *pValue)
{
  s_QtGLContext.pFunc->glGetUniformiv(pProgram->pProgram->programId(), (int)param, &pValue->x);
}

// ***************************************************************************************
void epShader_GetProgramData(epShaderProgram *pProgram, size_t param, ep::Vector3<int> *pValue)
{
  s_QtGLContext.pFunc->glGetUniformiv(pProgram->pProgram->programId(), (int)param, &pValue->x);
}

// ***************************************************************************************
void epShader_GetProgramData(epShaderProgram *pProgram, size_t param, ep::Vector4<int> *pValue)
{
  s_QtGLContext.pFunc->glGetUniformiv(pProgram->pProgram->programId(), (int)param, &pValue->x);
}

// ***************************************************************************************
void epShader_GetProgramData(epShaderProgram *pProgram, size_t param, ep::Vector2<uint32_t> *pValue)
{
  s_QtGLContext.pFunc->glGetUniformiv(pProgram->pProgram->programId(), (int)param, (int*)&pValue->x);
}

// ***************************************************************************************
void epShader_GetProgramData(epShaderProgram *pProgram, size_t param, ep::Vector3<uint32_t> *pValue)
{
  s_QtGLContext.pFunc->glGetUniformiv(pProgram->pProgram->programId(), (int)param, (int*)&pValue->x);
}

// ***************************************************************************************
void epShader_GetProgramData(epShaderProgram *pProgram, size_t param, ep::Vector4<uint32_t> *pValue)
{
  s_QtGLContext.pFunc->glGetUniformiv(pProgram->pProgram->programId(), (int)param, (int*)&pValue->x);
}

// ***************************************************************************************
void epShader_GetProgramData(epShaderProgram *pProgram, size_t param, ep::Float2 *pValue)
{
  s_QtGLContext.pFunc->glGetUniformfv(pProgram->pProgram->programId(), (int)param, &pValue->x);
}

// ***************************************************************************************
void epShader_GetProgramData(epShaderProgram *pProgram, size_t param, ep::Float3 *pValue)
{
  s_QtGLContext.pFunc->glGetUniformfv(pProgram->pProgram->programId(), (int)param, &pValue->x);
}

// ***************************************************************************************
void epShader_GetProgramData(epShaderProgram *pProgram, size_t param, ep::Float4 *pValue)
{
  s_QtGLContext.pFunc->glGetUniformfv(pProgram->pProgram->programId(), (int)param, &pValue->x);
}

// ***************************************************************************************
void epShader_GetProgramData(epShaderProgram *pProgram, size_t param, ep::Float4x4 *pValue)
{
  s_QtGLContext.pFunc->glGetUniformfv(pProgram->pProgram->programId(), (int)param, &pValue->a[0]);
}

// ***************************************************************************************
void epShader_GetProgramData(epShaderProgram *pProgram, size_t param, ep::Double2 *pValue)
{
  if (s_QtGLContext.pFunc4_0_Core)
    s_QtGLContext.pFunc4_0_Core->glGetUniformdv(pProgram->pProgram->programId(), (int)param, &pValue->x);
}

// ***************************************************************************************
void epShader_GetProgramData(epShaderProgram *pProgram, size_t param, ep::Double3 *pValue)
{
  if (s_QtGLContext.pFunc4_0_Core)
    s_QtGLContext.pFunc4_0_Core->glGetUniformdv(pProgram->pProgram->programId(), (int)param, &pValue->x);
}

// ***************************************************************************************
void epShader_GetProgramData(epShaderProgram *pProgram, size_t param, ep::Double4 *pValue)
{
  if (s_QtGLContext.pFunc4_0_Core)
    s_QtGLContext.pFunc4_0_Core->glGetUniformdv(pProgram->pProgram->programId(), (int)param, &pValue->x);
}

// ***************************************************************************************
void epShader_GetProgramData(epShaderProgram *pProgram, size_t param, ep::Double4x4 *pValue)
{
  if (s_QtGLContext.pFunc4_0_Core)
    s_QtGLContext.pFunc4_0_Core->glGetUniformdv(pProgram->pProgram->programId(), (int)param, &pValue->a[0]);
}

// The following 3 functions are for use with parameters that aren't part of our math lib.
// GL_FLOAT_MAT2, GL_FLOAT_MAT3, GL_FLOAT_MAT2x3, GL_FLOAT_MAT2x4, GL_FLOAT_MAT3x2, GL_FLOAT_MAT3x4, GL_FLOAT_MAT4x2,
// GL_FLOAT_MAT4x3, GL_INT_VEC2, GL_INT_VEC3, GL_INT_VEC4, GL_UNSIGNED_INT_VEC2, GL_UNSIGNED_INT_VEC3, GL_UNSIGNED_INT_VEC4,
// GL_DOUBLE_MAT2, GL_DOUBLE_MAT3, GL_DOUBLE_MAT2x3, GL_DOUBLE_MAT2x4, GL_DOUBLE_MAT3x2, GL_DOUBLE_MAT3x4, GL_DOUBLE_MAT4x2,
// GL_DOUBLE_MAT4x3
// The size is ignored as it can't be checked. Its purpose is just create an overload of the function.

// ***************************************************************************************
void epShader_GetProgramData(epShaderProgram *pProgram, size_t param, int *pValue, size_t)
{
  s_QtGLContext.pFunc->glGetUniformiv(pProgram->pProgram->programId(), (int)param, pValue);
}

// ***************************************************************************************
void epShader_GetProgramData(epShaderProgram *pProgram, size_t param, float *pValue, size_t)
{
  s_QtGLContext.pFunc->glGetUniformfv(pProgram->pProgram->programId(), (int)param, pValue);
}

// ***************************************************************************************
void epShader_GetProgramData(epShaderProgram *pProgram, size_t param, double *pValue, size_t)
{
  if (s_QtGLContext.pFunc4_0_Core)
    s_QtGLContext.pFunc4_0_Core->glGetUniformdv(pProgram->pProgram->programId(), (int)param, pValue);
}

// ***************************************************************************************
int epShader_FindShaderParameter(epShaderProgram *pProgram, const char *pName)
{
  // TODO: remove these checks once we are confident in Kernel and the Qt driver
  EPASSERT(pProgram->pProgram, "QOpenGLShaderProgram object not created");
  return pProgram->pProgram->uniformLocation(pName);
}

// ***************************************************************************************
void epShader_SetCurrent(epShaderProgram *pProgram)
{
  if (pProgram)
  {
    if (!pProgram->pProgram->bind())
      epDebugPrintf("Error binding shader program\n");
  }
  else
    s_QtGLContext.pFunc->glUseProgram(0);
}

// ***************************************************************************************
void epShader_SetProgramData(size_t param, bool value)
{
  s_QtGLContext.pFunc->glUniform1i((GLint)param, value ? 1 : 0);
}

// ***************************************************************************************
void epShader_SetProgramData(size_t param, const int *pValues, size_t count)
{
  s_QtGLContext.pFunc->glUniform1iv((GLint)param, (GLsizei)count, pValues);
}
void epShader_SetProgramData(size_t param, const ep::Vector2<int> *pValues, size_t count)
{
  s_QtGLContext.pFunc->glUniform2iv((GLint)param, (GLsizei)count, &pValues->x);
}
void epShader_SetProgramData(size_t param, const ep::Vector3<int> *pValues, size_t count)
{
  s_QtGLContext.pFunc->glUniform3iv((GLint)param, (GLsizei)count, &pValues->x);
}
void epShader_SetProgramData(size_t param, const ep::Vector4<int> *pValues, size_t count)
{
  s_QtGLContext.pFunc->glUniform4iv((GLint)param, (GLsizei)count, &pValues->x);
}

// ***************************************************************************************
void epShader_SetProgramData(size_t param, const uint32_t *pValues, size_t count)
{
  s_QtGLContext.pFunc->glUniform1iv((GLint)param, (GLsizei)count, (GLint*)pValues);
}
void epShader_SetProgramData(size_t param, const ep::Vector2<uint32_t> *pValues, size_t count)
{
  s_QtGLContext.pFunc->glUniform2iv((GLint)param, (GLsizei)count, (GLint*)&pValues->x);
}
void epShader_SetProgramData(size_t param, const ep::Vector3<uint32_t> *pValues, size_t count)
{
  s_QtGLContext.pFunc->glUniform3iv((GLint)param, (GLsizei)count, (GLint*)&pValues->x);
}
void epShader_SetProgramData(size_t param, const ep::Vector4<uint32_t> *pValues, size_t count)
{
  s_QtGLContext.pFunc->glUniform4iv((GLint)param, (GLsizei)count, (GLint*)&pValues->x);
}

// ***************************************************************************************
void epShader_SetProgramData(size_t param, const float *pValues, size_t count)
{
  s_QtGLContext.pFunc->glUniform1fv((GLint)param, (GLsizei)count, pValues);
}
void epShader_SetProgramData(size_t param, const ep::Vector2<float> *pValues, size_t count)
{
  s_QtGLContext.pFunc->glUniform2fv((GLint)param, (GLsizei)count, &pValues->x);
}
void epShader_SetProgramData(size_t param, const ep::Vector3<float> *pValues, size_t count)
{
  s_QtGLContext.pFunc->glUniform3fv((GLint)param, (GLsizei)count, &pValues->x);
}
void epShader_SetProgramData(size_t param, const ep::Vector4<float> *pValues, size_t count)
{
  s_QtGLContext.pFunc->glUniform4fv((GLint)param, (GLsizei)count, &pValues->x);
}
void epShader_SetProgramData(size_t param, const ep::Matrix4x4<float> *pValues, size_t count)
{
  s_QtGLContext.pFunc->glUniformMatrix4fv((GLint)param, (GLsizei)count, (GLboolean)false, &pValues->a[0]);
}

// ***************************************************************************************
void epShader_SetProgramData(size_t param, const double *pValues, size_t count)
{
  if (s_QtGLContext.pFunc4_0_Core)
    s_QtGLContext.pFunc4_0_Core->glUniform1dv((GLint)param, (GLsizei)count, pValues);
}
void epShader_SetProgramData(size_t param, const ep::Vector2<double> *pValues, size_t count)
{
  if (s_QtGLContext.pFunc4_0_Core)
    s_QtGLContext.pFunc4_0_Core->glUniform2dv((GLint)param, (GLsizei)count, &pValues->x);
}
void epShader_SetProgramData(size_t param, const ep::Vector3<double> *pValues, size_t count)
{
  if (s_QtGLContext.pFunc4_0_Core)
    s_QtGLContext.pFunc4_0_Core->glUniform3dv((GLint)param, (GLsizei)count, &pValues->x);
}
void epShader_SetProgramData(size_t param, const ep::Vector4<double> *pValues, size_t count)
{
  if (s_QtGLContext.pFunc4_0_Core)
    s_QtGLContext.pFunc4_0_Core->glUniform4dv((GLint)param, (GLsizei)count, &pValues->x);
}
void epShader_SetProgramData(size_t param, const ep::Matrix4x4<double> *pValues, size_t count)
{
  if (s_QtGLContext.pFunc4_0_Core)
    s_QtGLContext.pFunc4_0_Core->glUniformMatrix4dv((GLint)param, (GLsizei)count, (GLboolean)false, &pValues->a[0]);
}

// ***************************************************************************************
void epShader_SetProgramData(size_t textureUnit, size_t param, struct epTexture *pTexture)
{
  pTexture->pTexture->bind((int)textureUnit);

  // TODO: Use QOpenGLShaderProgram instead?
  s_QtGLContext.pFunc->glUniform1i((int)param, (int)textureUnit);
}

#else
EPEMPTYFILE
#endif // EPRENDER_DRIVER == EPDRIVER_QT
