#include "hal/driver.h"

#if EPRENDER_DRIVER == EPDRIVER_OPENGL

#include "hal/shader.h"
#include "hal/texture.h"
#include "hal/vertex.h"

#include "ep_opengl.h"


static GLenum s_shaderType[epST_Max] =
{
  GL_VERTEX_SHADER,
  GL_FRAGMENT_SHADER
};

extern GLenum s_textureType[epTT_Max];


// ***************************************************************************************
// Author: Manu Evans, May 2015
epShader* epShader_CreateShaderFromFile(const char *pFilename, epShaderType type)
{
  epUnused(pFilename);
  // load file
  //...
  GLchar *pSource = nullptr;

  return epShader_CreateShader(pSource, 0, type);
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
epShader* epShader_CreateShader(const char *pSource, size_t length, epShaderType type)
{
  GLuint shader = glCreateShader(s_shaderType[type]);

  GLint len = (GLint)length;
  glShaderSource(shader, 1, &pSource, len ? &len : nullptr);

  glCompileShader(shader);

  GLint compile_status;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_status);
  if (compile_status != GL_TRUE)
  {
    // Shader failed to compile, let's see what the error is.
    char buffer[1024];
    GLsizei length;
    glGetShaderInfoLog(shader, sizeof(buffer), &length, &buffer[0]);
    udDebugPrintf("Shader failed to compile: %s\n", buffer);
    return 0;
  }

  epShader *pShader = epAllocType(epShader, 1, epAF_None);
  pShader->shader = shader;

  return pShader;
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
epShaderProgram* epShader_CreateShaderProgram(epShader *pVertexShader, epShader *pPixelShader)
{
  GLuint program = glCreateProgram();
  glAttachShader(program, pPixelShader->shader);
  glAttachShader(program, pVertexShader->shader);
  glLinkProgram(program);

  GLint link_status;
  glGetProgramiv(program, GL_LINK_STATUS, &link_status);
  if (link_status != GL_TRUE)
  {
    // Program failed to link, let's see what the error is.
    char buffer[1024];
    GLsizei length;
    glGetProgramInfoLog(program, sizeof(buffer), &length, &buffer[0]);
    udDebugPrintf("Program failed to link: %s\n", buffer);
    return 0;
  }

  epShaderProgram *pProgram = epAllocType(epShaderProgram, 1, epAF_None);
  pProgram->program = program;

  size_t extraBytes = 0;

  GLint numAttributes;
  glGetProgramiv(program, GL_ACTIVE_ATTRIBUTES, &numAttributes);
  for (GLint i = 0; i < numAttributes; ++i)
  {
    char name[128];
    GLsizei length;
    GLint size;
    GLenum type;
    glGetActiveAttrib(program, i, sizeof(name), &length, &size, &type, name);
    extraBytes += length + 1;
  }

  GLint numUniforms;
  glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &numUniforms);
  for (GLint i = 0; i < numUniforms; ++i)
  {
    char name[128];
    GLsizei length;
    GLint size;
    GLenum type;
    glGetActiveUniform(program, i, sizeof(name), &length, &size, &type, name);
    extraBytes += length + 1;
  }

  extraBytes += sizeof(epShaderProgram::Param) * (numAttributes + numUniforms);

  pProgram = (epShaderProgram*)epAlloc(sizeof(epShaderProgram) + extraBytes);
  pProgram->program = program;
  pProgram->numAttributes = numAttributes;
  pProgram->numUniforms = numUniforms;
  pProgram->pAttributes = (epShaderProgram::Param*)&pProgram[1];
  pProgram->pUniforms = (epShaderProgram::Param*)&pProgram->pAttributes[numAttributes];
  char *pStrings = (char*)&pProgram->pUniforms[numUniforms];

  for (GLint i = 0; i < numAttributes; ++i)
  {
    GLsizei length;
    GLint size;
    GLenum type;
    pProgram->pAttributes[i].pName = pStrings;
    glGetActiveAttrib(program, i, 128, &length, &size, &type, pProgram->pAttributes[i].pName);
    pProgram->pAttributes[i].type = type;
    pStrings += length + 1;
  }

  for (GLint i = 0; i < numUniforms; ++i)
  {
    GLsizei length;
    GLint size;
    GLenum type;
    pProgram->pUniforms[i].pName = pStrings;
    glGetActiveUniform(program, i, 128, &length, &size, &type, pProgram->pUniforms[i].pName);
    pProgram->pUniforms[i].type = type;
    pStrings += length + 1;
  }


  return pProgram;
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
size_t epShader_GetAttributeType(epShaderProgram *pProgram, size_t i)
{
  return pProgram->pAttributes[i].type;
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
size_t epShader_GetUniformType(epShaderProgram *pProgram, size_t i)
{
  return pProgram->pUniforms[i].type;
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
int epShader_FindShaderParameter(epShaderProgram *pProgram, const char *pName)
{
  return (int)glGetUniformLocation(pProgram->program, pName);
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
void epShader_SetCurrent(epShaderProgram *pProgram)
{
  if(pProgram)
    glUseProgram(pProgram->program);
  else
    glUseProgram(0);
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
void epShader_SetProgramData(int param, bool value)
{
  if(param < 0)
    return;
  glUniform1i(param, value ? 1 : 0);
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
void epShader_SetProgramData(int param, int value)
{
  if(param < 0)
    return;
  glUniform1i(param, value);
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
void epShader_SetProgramData(int param, float value)
{
  if(param < 0)
    return;
  glUniform1f(param, value);
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
void epShader_SetProgramData(int param, const ep::Float4 &value)
{
  if(param < 0)
    return;
  glUniform4fv(param, 1, (GLfloat*)&value);
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
void epShader_SetProgramData(int param, const ep::Float4x4 &value)
{
  if(param < 0)
    return;
  glUniformMatrix4fv(param, 1, 0, (GLfloat*)&value);
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
void epShader_SetProgramData(int textureUnit, int param, struct epTexture *pTexture)
{
  if(param < 0)
    return;
  glActiveTexture(GL_TEXTURE0 + textureUnit);
  glBindTexture(s_textureType[pTexture->type], pTexture->texture);
  glUniform1i(param, textureUnit);
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
void epShader_SetProgramData(int param, const int *pValues, size_t count)
{
  if(param < 0)
    return;
  glUniform1iv(param, (GLsizei)count, pValues);
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
void epShader_SetProgramData(int param, const float *pValues, size_t count)
{
  if(param < 0)
    return;
  glUniform1fv(param, (GLsizei)count, pValues);
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
void epShader_SetProgramData(int param, const ep::Float4 *pValues, size_t count)
{
  if(param < 0)
    return;
  glUniform4fv(param, (GLsizei)count, (GLfloat*)pValues);
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
void epShader_SetProgramData(int param, const ep::Float4x4 *pValues, size_t count)
{
  if(param < 0)
    return;
  glUniformMatrix4fv(param, (GLsizei)count, 0, (GLfloat*)pValues);
}

#else
EPEMPTYFILE
#endif // EPRENDER_DRIVER == EPDRIVER_OPENGL
