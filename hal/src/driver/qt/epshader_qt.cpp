#include "driver.h"

#if EPRENDER_DRIVER == EPDRIVER_QT

#include "hal/shader.h"
#include "hal/vertex.h"

#include "eprender_qt.h"

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
  EPASSERT(ppShader && *ppShader, "ppShader is invalid");
  epShader *pShader = *ppShader;
  delete pShader->pShader;
  epFree(pShader);
  *ppShader = nullptr;
}

// ***************************************************************************************
static inline epShaderElement ConvertToShaderElementType(size_t type, uint32_t location)
{
  epShaderElement uniformType = { 0, 0 , 0, location, 0, 0, 0 };
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
    case GL_SAMPLER_2D:
    {
      uniformType.m = 1;
      uniformType.n = 1;
      uniformType.type = epSET_Float; // TODO: find element types
      uniformType.samplerType = epSST_Default; // TODO: find texture type
      uniformType.samplerDimensions = epSSD_2D; // TODO: find texture dimensions
      break;
    }
  }

  return uniformType;
}

// ***************************************************************************************
epShaderProgram* epShader_CreateShaderProgram(epShader *shaders[], size_t arrayLength)
{
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
      pProgram->pAttributes[i].elementType = ConvertToShaderElementType(type, pQtProgram->attributeLocation(pProgram->pAttributes[i].pName));
      pStrings += length + 1;
    }

    for (GLint i = 0; i < numUniforms; ++i)
    {
      GLsizei length;
      GLint size;
      GLenum type;
      pProgram->pUniforms[i].pName = pStrings;
      funcs.glGetActiveUniform(program, i, pEnd - pStrings, &length, &size, &type, pProgram->pUniforms[i].pName);
      pProgram->pUniforms[i].elementType = ConvertToShaderElementType(type, pQtProgram->uniformLocation(pProgram->pUniforms[i].pName));
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
        memcpy(pBuffer, "sampler", 7);
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

  size_t length = 3;

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
  s_QtGLContext.pFunc->glUniform1i((int)param, value ? 1 : 0);
}

// ***************************************************************************************
void epShader_SetProgramData(size_t param, int value)
{
  s_QtGLContext.pFunc->glUniform1i((int)param, value);
}

// ***************************************************************************************
void epShader_SetProgramData(size_t param, uint32_t value)
{
   s_QtGLContext.pFunc->glUniform1i((int)param, (int)value);
}

// ***************************************************************************************
void epShader_SetProgramData(size_t param, float value)
{
  if (s_QtGLContext.pFunc4_0_Core)
    s_QtGLContext.pFunc4_0_Core->glUniform1fv((int)param, 1, &value);
}

// ***************************************************************************************
void epShader_SetProgramData(size_t param, double value)
{
  if (s_QtGLContext.pFunc4_0_Core)
    s_QtGLContext.pFunc4_0_Core->glUniform1dv((int)param, 1, &value);
}

// ***************************************************************************************
void epShader_SetProgramData(size_t param, const ep::Vector2<int> &value)
{
  s_QtGLContext.pFunc->glUniform2iv((int)param, 1, &value.x);
}

// ***************************************************************************************
void epShader_SetProgramData(size_t param, const ep::Vector3<int> &value)
{
  s_QtGLContext.pFunc->glUniform3iv((int)param, 1, &value.x);
}

// ***************************************************************************************
void epShader_SetProgramData(size_t param, const ep::Vector4<int> &value)
{
  s_QtGLContext.pFunc->glUniform4iv((int)param, 1, &value.x);
}

// ***************************************************************************************
void epShader_SetProgramData(size_t param, const ep::Vector2<uint32_t> &value)
{
  s_QtGLContext.pFunc->glUniform2iv((int)param, 1, (int*)&value.x);
}

// ***************************************************************************************
void epShader_SetProgramData(size_t param, const ep::Vector3<uint32_t> &value)
{
  s_QtGLContext.pFunc->glUniform3iv((int)param, 1, (int*)&value.x);
}

// ***************************************************************************************
void epShader_SetProgramData(size_t param, const ep::Vector4<uint32_t> &value)
{
  s_QtGLContext.pFunc->glUniform4iv((int)param, 1, (int*)&value.x);
}

// ***************************************************************************************
void epShader_SetProgramData(size_t param, const ep::Float2 &value)
{
  s_QtGLContext.pFunc->glUniform2fv((int)param, 1, &value.x);
}

// ***************************************************************************************
void epShader_SetProgramData(size_t param, const ep::Float3 &value)
{
  s_QtGLContext.pFunc->glUniform3fv((int)param, 1, &value.x);
}

// ***************************************************************************************
void epShader_SetProgramData(size_t param, const ep::Float4 &value)
{
  s_QtGLContext.pFunc->glUniform4fv((int)param, 1, &value.x);
}

// ***************************************************************************************
void epShader_SetProgramData(size_t param, const ep::Float4x4 &value)
{
  s_QtGLContext.pFunc->glUniformMatrix4fv((int)param, 1, 0, &value.a[0]);
}

// ***************************************************************************************
void epShader_SetProgramData(size_t param, const ep::Double2 &value)
{
  if (s_QtGLContext.pFunc4_0_Core)
    s_QtGLContext.pFunc4_0_Core->glUniform2dv((int)param, 1, &value.x);
}

// ***************************************************************************************
void epShader_SetProgramData(size_t param, const ep::Double3 &value)
{
  if (s_QtGLContext.pFunc4_0_Core)
    s_QtGLContext.pFunc4_0_Core->glUniform3dv((int)param, 1, &value.x);
}

// ***************************************************************************************
void epShader_SetProgramData(size_t param, const ep::Double4 &value)
{
  if (s_QtGLContext.pFunc4_0_Core)
    s_QtGLContext.pFunc4_0_Core->glUniform4dv((int)param, 1, &value.x);
}

// ***************************************************************************************
void epShader_SetProgramData(size_t param, const ep::Double4x4 &value)
{
  if (s_QtGLContext.pFunc4_0_Core)
    s_QtGLContext.pFunc4_0_Core->glUniformMatrix4dv((int)param, 1, GLboolean(0), &value.a[0]);
}

// ***************************************************************************************
void epShader_SetProgramData(size_t textureUnit, size_t param, struct epTexture *pTexture)
{
  pTexture->pTexture->bind((int)textureUnit);

  // TODO: Use QOpenGLShaderProgram instead?
  s_QtGLContext.pFunc->glUniform1i((int)param, (int)textureUnit);
}

// ***************************************************************************************
void epShader_SetProgramData(size_t param, const int *pValues, size_t count)
{
  s_QtGLContext.pFunc->glUniform1iv((int)param, (GLsizei)count, pValues);
}

// ***************************************************************************************
void epShader_SetProgramData(size_t param, const float *pValues, size_t count)
{
  s_QtGLContext.pFunc->glUniform1fv((int)param, (GLsizei)count, pValues);
}

// ***************************************************************************************
void epShader_SetProgramData(size_t param, const double *pValue, size_t count)
{
  if (s_QtGLContext.pFunc4_0_Core)
    s_QtGLContext.pFunc4_0_Core->glUniform1dv((int)param, (GLsizei)count, pValue);
}

// ***************************************************************************************
void epShader_SetProgramData(size_t param, const ep::Float2 *pValues, size_t count)
{
  s_QtGLContext.pFunc->glUniform2fv((int)param, (GLsizei)count, &pValues->x);
}

// ***************************************************************************************
void epShader_SetProgramData(size_t param, const ep::Float3 *pValues, size_t count)
{
  s_QtGLContext.pFunc->glUniform3fv((int)param, (GLsizei)count, &pValues->x);
}

// ***************************************************************************************
void epShader_SetProgramData(size_t param, const ep::Float4 *pValues, size_t count)
{
  s_QtGLContext.pFunc->glUniform4fv((int)param, (GLsizei)count, &pValues->x);
}

// ***************************************************************************************
void epShader_SetProgramData(size_t param, const ep::Float4x4 *pValues, size_t count)
{
  if (s_QtGLContext.pFunc4_0_Core)
    s_QtGLContext.pFunc->glUniformMatrix4fv((int)param, (GLsizei)count, 0, &pValues->a[0]);
}

// ***************************************************************************************
void epShader_SetProgramData(size_t param, const ep::Double2 *pValues, size_t count)
{
  if (s_QtGLContext.pFunc4_0_Core)
    s_QtGLContext.pFunc4_0_Core->glUniform2dv((int)param, (GLsizei)count, &pValues->x);
}

// ***************************************************************************************
void epShader_SetProgramData(size_t param, const ep::Double3 *pValues, size_t count)
{
  if (s_QtGLContext.pFunc4_0_Core)
    s_QtGLContext.pFunc4_0_Core->glUniform3dv((int)param, (GLsizei)count, &pValues->x);
}

// ***************************************************************************************
void epShader_SetProgramData(size_t param, const ep::Double4 *pValues, size_t count)
{
  if (s_QtGLContext.pFunc4_0_Core)
    s_QtGLContext.pFunc4_0_Core->glUniform4dv((int)param, (GLsizei)count, &pValues->x);
}

// ***************************************************************************************
void epShader_SetProgramData(size_t param, const ep::Double4x4 *pValues, size_t count)
{
  if (s_QtGLContext.pFunc4_0_Core)
    s_QtGLContext.pFunc4_0_Core->glUniformMatrix4dv((int)param, (GLsizei)count, 0, &pValues->a[0]);
}

#else
EPEMPTYFILE
#endif // EPRENDER_DRIVER == EPDRIVER_QT
