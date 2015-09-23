#include "hal/driver.h"

#if UDRENDER_DRIVER == UDDRIVER_QT

#include "hal/shader.h"
#include "hal/vertex.h"

#include "eprender_qt.h"

#include <QOpenGLShaderProgram>
#include <QOpenGLShader>
#include <QOpenGLTexture>
#include <QOpenGLFunctions>

static QOpenGLShader::ShaderTypeBit s_shaderType[udST_Max] =
{
  QOpenGLShader::Vertex,
  QOpenGLShader::Fragment
};


// ***************************************************************************************
udShader* udShader_CreateShaderFromFile(const char *udUnusedParam(pFilename), udShaderType udUnusedParam(type))
{
  UDASSERT(false, "TODO");
  return 0;
}

// ***************************************************************************************
udShader* udShader_CreateShader(const char *pSource, size_t length, udShaderType type)
{
  QOpenGLShader *pQtShader = new QOpenGLShader(s_shaderType[type]);
  if (!pQtShader->compileSourceCode(QByteArray(pSource, static_cast<int>(length))))
  {
    udDebugPrintf("Shader failed to compile: %s\n", pQtShader->log().toLatin1().data());
    delete pQtShader;
    return 0;
  }

  udShader *pShader = udAllocType(udShader, 1, udAF_None);
  pShader->pShader = pQtShader;
  return pShader;
}

// ***************************************************************************************
udShaderProgram* udShader_CreateShaderProgram(udShader *pVertexShader, udShader *pPixelShader)
{
  bool result = true;
  udShaderProgram *pProgram = nullptr;

  QOpenGLShaderProgram *pQtProgram = new QOpenGLShaderProgram();
  UD_ERROR_IF(!pQtProgram->addShader(pVertexShader->pShader), false);
  UD_ERROR_IF(!pQtProgram->addShader(pPixelShader->pShader), false);
  UD_ERROR_IF(!pQtProgram->link(), false);

  {
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

    extraBytes += sizeof(udShaderProgram::Param) * (numAttributes + numUniforms);

    pProgram = (udShaderProgram*)udAlloc(sizeof(udShaderProgram) + extraBytes);
    pProgram->pProgram = pQtProgram;
    pProgram->numAttributes = numAttributes;
    pProgram->numUniforms = numUniforms;
    pProgram->pAttributes = (udShaderProgram::Param*)&pProgram[1];
    pProgram->pUniforms = (udShaderProgram::Param*)&pProgram->pAttributes[numAttributes];
    char *pStrings = (char*)&pProgram->pUniforms[numUniforms];
    char *pEnd = (char*)&pProgram[1] + extraBytes;

    for (GLint i = 0; i < numAttributes; ++i)
    {
      GLsizei length;
      GLint size;
      GLenum type;
      pProgram->pAttributes[i].pName = pStrings;
      funcs.glGetActiveAttrib(program, i, pEnd - pStrings, &length, &size, &type, pProgram->pAttributes[i].pName);
      pProgram->pAttributes[i].type = type;
      pStrings += length + 1;
    }

    for (GLint i = 0; i < numUniforms; ++i)
    {
      GLsizei length;
      GLint size;
      GLenum type;
      pProgram->pUniforms[i].pName = pStrings;
      funcs.glGetActiveUniform(program, i, pEnd - pStrings, &length, &size, &type, pProgram->pUniforms[i].pName);
      pProgram->pUniforms[i].type = type;
      pStrings += length + 1;
    }
  }

epilogue:
  if (!result)
  {
    udDebugPrintf("Error creating shader program: %s\n", pQtProgram->log().toLatin1().data());
    delete pQtProgram;
  }

  return pProgram;
}

// ***************************************************************************************
size_t udShader_GetNumAttributes(udShaderProgram *pProgram)
{
  return pProgram->numAttributes;
}

// ***************************************************************************************
const char *udShader_GetAttributeName(udShaderProgram *pProgram, size_t i)
{
  return pProgram->pAttributes[i].pName;
}

// ***************************************************************************************
size_t udShader_GetAttributeType(udShaderProgram *pProgram, size_t i)
{
  return pProgram->pAttributes[i].type;
}

// ***************************************************************************************
size_t udShader_GetNumUniforms(udShaderProgram *pProgram)
{
  return pProgram->numUniforms;
}

// ***************************************************************************************
const char *udShader_GetUniformName(udShaderProgram *pProgram, size_t i)
{
  return pProgram->pUniforms[i].pName;
}

// ***************************************************************************************
size_t udShader_GetUniformType(udShaderProgram *pProgram, size_t i)
{
  return pProgram->pUniforms[i].type;
}

// ***************************************************************************************
int udShader_FindShaderParameter(udShaderProgram *pProgram, const char *pName)
{
  // TODO: remove these checks once we are confident in Kernel and the Qt driver
  UDASSERT(pProgram->pProgram, "QOpenGLShaderProgram object not created");
  return pProgram->pProgram->uniformLocation(pName);
}

// ***************************************************************************************
void udShader_SetCurrent(udShaderProgram *pProgram)
{
  if (pProgram)
  {
    if (!pProgram->pProgram->bind())
      udDebugPrintf("Error binding shader program");
  }
  else
    s_QtGLContext.pFunc->glUseProgram(0);
}

// ***************************************************************************************
void udShader_SetProgramData(int param, bool value)
{
  if (param < 0)
    return;
  s_QtGLContext.pFunc->glUniform1i(param, value ? 1 : 0);
}

// ***************************************************************************************
void udShader_SetProgramData(int param, int value)
{
  if (param < 0)
    return;
  s_QtGLContext.pFunc->glUniform1i(param, value);
}

// ***************************************************************************************
void udShader_SetProgramData(int param, float value)
{
  if (param < 0)
    return;
  s_QtGLContext.pFunc->glUniform1f(param, value);
}

// ***************************************************************************************
void udShader_SetProgramData(int param, const udFloat4 &value)
{
  if (param < 0)
    return;
  s_QtGLContext.pFunc->glUniform4fv(param, 1, (GLfloat*)&value);
}

// ***************************************************************************************
void udShader_SetProgramData(int param, const udFloat4x4 &value)
{
  if (param < 0)
    return;
  s_QtGLContext.pFunc->glUniformMatrix4fv(param, 1, 0, (GLfloat*)&value);
}

// ***************************************************************************************
void udShader_SetProgramData(int textureUnit, int param, struct udTexture *pTexture)
{
  if (param < 0)
    return;

  pTexture->pTexture->bind(textureUnit);

  // TODO: Use QOpenGLShaderProgram instead?
  s_QtGLContext.pFunc->glUniform1i(param, textureUnit);
}

// ***************************************************************************************
void udShader_SetProgramData(int param, const int *pValues, size_t count)
{
  if (param < 0)
    return;
  s_QtGLContext.pFunc->glUniform1iv(param, (GLsizei)count, pValues);
}

// ***************************************************************************************
void udShader_SetProgramData(int param, const float *pValues, size_t count)
{
  if (param < 0)
    return;
  s_QtGLContext.pFunc->glUniform1fv(param, (GLsizei)count, pValues);
}

// ***************************************************************************************
void udShader_SetProgramData(int param, const udFloat4 *pValues, size_t count)
{
  if (param < 0)
    return;
  s_QtGLContext.pFunc->glUniform4fv(param, (GLsizei)count, (GLfloat*)pValues);
}

// ***************************************************************************************
void udShader_SetProgramData(int param, const udFloat4x4 *pValues, size_t count)
{
  if (param < 0)
    return;
  s_QtGLContext.pFunc->glUniformMatrix4fv(param, (GLsizei)count, 0, (GLfloat*)pValues);
}

#endif // UDRENDER_DRIVER == UDDRIVER_QT
