#include "driver.h"

#if EPRENDER_DRIVER == EPDRIVER_OPENGL

#include "hal/shader.h"
#include "hal/texture.h"
#include "hal/vertex.h"

#include "ep_opengl.h"


static GLenum s_shaderType[epST_Max] =
{
  GL_VERTEX_SHADER,
  GL_FRAGMENT_SHADER,
  GL_GEOMETRY_SHADER,
  GL_TESS_CONTROL_SHADER,
  GL_TESS_EVALUATION_SHADER,
  GL_COMPUTE_SHADER
};

extern GLenum s_textureType[epTT_Max];


// ***************************************************************************************
// Author: Manu Evans, May 2015
epShader* epShader_CreateShaderFromFile(const char *pFilename, epShaderType type)
{
  epUnused(pFilename);

  EPASSERT(false, "Not implemented");
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
    GLsizei _length;
    glGetShaderInfoLog(shader, sizeof(buffer), &_length, &buffer[0]);
    epDebugPrintf("Shader failed to compile: %s\n", buffer);
    glDeleteShader(shader);
    return nullptr;
  }

  epShader *pShader = epAllocType(epShader, 1, epAF_None);
  pShader->shader = shader;

  return pShader;
}

// ***************************************************************************************
// Author: David Ely, February 2016
void epShader_DestroyShader(epShader **ppShader)
{
  EPASSERT(ppShader && *ppShader, "ppShader is invalid");

  epShader *pShader = *ppShader;
  glDeleteShader(pShader->shader);
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
// Author: Manu Evans, May 2015
epShaderProgram* epShader_CreateShaderProgram(epShader *shaders[], size_t arrayLength)
{
  // TODO: Error handling
  GLuint program = glCreateProgram();
  for (size_t i = 0; i < arrayLength; ++i)
    glAttachShader(program, shaders[i]->shader);
  glLinkProgram(program);

  GLint link_status;
  glGetProgramiv(program, GL_LINK_STATUS, &link_status);
  if (link_status != GL_TRUE)
  {
    // Program failed to link, let's see what the error is.
    char buffer[1024];
    GLsizei length;
    glGetProgramInfoLog(program, sizeof(buffer), &length, &buffer[0]);
    epDebugPrintf("Program failed to link: %s\n", buffer);
    glDeleteProgram(program);
    return nullptr;
  }

  //Always detach shaders after a successful link.
  for (size_t i = 0; i < arrayLength; ++i)
    glDetachShader(program, shaders[i]->shader);

  glUseProgram(program);

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

  epShaderProgram *pProgram = (epShaderProgram*)epAlloc(sizeof(epShaderProgram) + extraBytes);
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
    pProgram->pAttributes[i].elementType = ConvertToShaderElementType(type, glGetAttribLocation(program, pProgram->pAttributes[i].pName));
    pStrings += length + 1;
  }

  for (GLint i = 0; i < numUniforms; ++i)
  {
    GLsizei length;
    GLint size;
    GLenum type;
    pProgram->pUniforms[i].pName = pStrings;
    glGetActiveUniform(program, i, 128, &length, &size, &type, pProgram->pUniforms[i].pName);
    pProgram->pUniforms[i].elementType = ConvertToShaderElementType(type, glGetUniformLocation(program, pProgram->pUniforms[i].pName));
    pStrings += length + 1;
  }

  return pProgram;
}

void epShader_DestroyShaderProgram(epShaderProgram **ppProgram)
{
  EPASSERT(ppProgram && *ppProgram, "ppProgram is invalid");
  epShaderProgram *pProgram = *ppProgram;
  glDeleteProgram(pProgram->program);
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
  glGetUniformiv(pProgram->program, (int)param, &glInt);
  *pValue = glInt != 0;
}

// ***************************************************************************************
void epShader_GetProgramData(epShaderProgram *pProgram, size_t param, int *pValue)
{
  glGetUniformiv(pProgram->program, (int)param, pValue);
}

// ***************************************************************************************
void epShader_GetProgramData(epShaderProgram *pProgram, size_t param, uint32_t *pValue)
{
  glGetUniformiv(pProgram->program, (int)param, (int*)pValue);
}

// ***************************************************************************************
void epShader_GetProgramData(epShaderProgram *pProgram, size_t param, float *pValue)
{
  GLfloat glFloat;
  glGetUniformfv(pProgram->program, (int)param, &glFloat);

  *pValue = (float)glFloat;
}

// ***************************************************************************************
void epShader_GetProgramData(epShaderProgram *pProgram, size_t param, double *pValue)
{
    glGetUniformdv(pProgram->program, (int)param, pValue);
}

// ***************************************************************************************
void epShader_GetProgramData(epShaderProgram *pProgram, size_t param, ep::Vector2<int> *pValue)
{
  glGetUniformiv(pProgram->program, (int)param, &pValue->x);
}

// ***************************************************************************************
void epShader_GetProgramData(epShaderProgram *pProgram, size_t param, ep::Vector3<int> *pValue)
{
  glGetUniformiv(pProgram->program, (int)param, &pValue->x);
}

// ***************************************************************************************
void epShader_GetProgramData(epShaderProgram *pProgram, size_t param, ep::Vector4<int> *pValue)
{
  glGetUniformiv(pProgram->program, (int)param, &pValue->x);
}

// ***************************************************************************************
void epShader_GetProgramData(epShaderProgram *pProgram, size_t param, ep::Vector2<uint32_t> *pValue)
{
  glGetUniformiv(pProgram->program, (int)param, (int*)&pValue->x);
}

// ***************************************************************************************
void epShader_GetProgramData(epShaderProgram *pProgram, size_t param, ep::Vector3<uint32_t> *pValue)
{
  glGetUniformiv(pProgram->program, (int)param, (int*)&pValue->x);
}

// ***************************************************************************************
void epShader_GetProgramData(epShaderProgram *pProgram, size_t param, ep::Vector4<uint32_t> *pValue)
{
  glGetUniformiv(pProgram->program, (int)param, (int*)&pValue->x);
}

// ***************************************************************************************
void epShader_GetProgramData(epShaderProgram *pProgram, size_t param, ep::Float2 *pValue)
{
  glGetUniformfv(pProgram->program, (int)param, &pValue->x);
}

// ***************************************************************************************
void epShader_GetProgramData(epShaderProgram *pProgram, size_t param, ep::Float3 *pValue)
{
  glGetUniformfv(pProgram->program, (int)param, &pValue->x);
}

// ***************************************************************************************
void epShader_GetProgramData(epShaderProgram *pProgram, size_t param, ep::Float4 *pValue)
{
  glGetUniformfv(pProgram->program, (int)param, &pValue->x);
}

// ***************************************************************************************
void epShader_GetProgramData(epShaderProgram *pProgram, size_t param, ep::Float4x4 *pValue)
{
  glGetUniformfv(pProgram->program, (int)param, &pValue->a[0]);
}

// ***************************************************************************************
void epShader_GetProgramData(epShaderProgram *pProgram, size_t param, ep::Double2 *pValue)
{
  glGetUniformdv(pProgram->program, (int)param, &pValue->x);
}

// ***************************************************************************************
void epShader_GetProgramData(epShaderProgram *pProgram, size_t param, ep::Double3 *pValue)
{
  glGetUniformdv(pProgram->program, (int)param, &pValue->x);
}

// ***************************************************************************************
void epShader_GetProgramData(epShaderProgram *pProgram, size_t param, ep::Double4 *pValue)
{
  glGetUniformdv(pProgram->program, (int)param, &pValue->x);
}

// ***************************************************************************************
void epShader_GetProgramData(epShaderProgram *pProgram, size_t param, ep::Double4x4 *pValue)
{
  glGetUniformdv(pProgram->program, (int)param, &pValue->a[0]);
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
  glGetUniformiv(pProgram->program, (int)param, pValue);
}

// ***************************************************************************************
void epShader_GetProgramData(epShaderProgram *pProgram, size_t param, float *pValue, size_t)
{
  glGetUniformfv(pProgram->program, (int)param, pValue);
}

// ***************************************************************************************
void epShader_GetProgramData(epShaderProgram *pProgram, size_t param, double *pValue, size_t)
{
  glGetUniformdv(pProgram->program, (int)param, pValue);
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
void epShader_SetProgramData(size_t param, bool value)
{
  glUniform1i((int)param, value ? 1 : 0);
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
void epShader_SetProgramData(size_t param, int value)
{
  glUniform1i((int)param, value);
}

// ***************************************************************************************
void epShader_SetProgramData(size_t param, uint32_t value)
{
  glUniform1i((int)param, (int)value);
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
void epShader_SetProgramData(size_t param, float value)
{
  glUniform1f((int)param, value);
}

// ***************************************************************************************
void epShader_SetProgramData(size_t param, double value)
{
  glUniform1dv((int)param, 1, &value);
}

// ***************************************************************************************
void epShader_SetProgramData(size_t param, const ep::Vector2<int> &value)
{
  glUniform2iv((int)param, 1, &value.x);
}

// ***************************************************************************************
void epShader_SetProgramData(size_t param, const ep::Vector3<int> &value)
{
  glUniform3iv((int)param, 1, &value.x);
}

// ***************************************************************************************
void epShader_SetProgramData(size_t param, const ep::Vector4<int> &value)
{
  glUniform4iv((int)param, 1, &value.x);
}

// ***************************************************************************************
void epShader_SetProgramData(size_t param, const ep::Vector2<uint32_t> &value)
{
  glUniform2iv((int)param, 1, (int*)&value.x);
}

// ***************************************************************************************
void epShader_SetProgramData(size_t param, const ep::Vector3<uint32_t> &value)
{
  glUniform3iv((int)param, 1, (int*)&value.x);
}

// ***************************************************************************************
void epShader_SetProgramData(size_t param, const ep::Vector4<uint32_t> &value)
{
  glUniform4iv((int)param, 1, (int*)&value.x);
}

// ***************************************************************************************
void epShader_SetProgramData(size_t param, const ep::Float2 &value)
{
  glUniform2fv((int)param, 1, &value.x);
}

// ***************************************************************************************
void epShader_SetProgramData(size_t param, const ep::Float3 &value)
{
  glUniform3fv((int)param, 1, &value.x);
}

// ***************************************************************************************
void epShader_SetProgramData(size_t param, const ep::Float4 &value)
{
  glUniform4fv((int)param, 1, &value.x);
}

// ***************************************************************************************
void epShader_SetProgramData(size_t param, const ep::Double2 &value)
{
  glUniform2dv((int)param, 1, &value.x);
}

// ***************************************************************************************
void epShader_SetProgramData(size_t param, const ep::Double3 &value)
{
  glUniform3dv((int)param, 1, &value.x);
}

// ***************************************************************************************
void epShader_SetProgramData(size_t param, const ep::Double4 &value)
{
  glUniform4dv((int)param, 1, &value.x);
}

// ***************************************************************************************
void epShader_SetProgramData(size_t param, const ep::Double4x4 &value)
{
  glUniformMatrix4dv((int)param, 1, GLboolean(0), &value.a[0]);
}
// ***************************************************************************************
// Author: Manu Evans, May 2015
void epShader_SetProgramData(size_t param, const ep::Float4x4 &value)
{
  glUniformMatrix4fv((int)param, 1, 0, &value.a[0]);
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
void epShader_SetProgramData(size_t textureUnit, size_t param, struct epTexture *pTexture)
{
  glActiveTexture(GL_TEXTURE0 + (int)textureUnit);
  glBindTexture(s_textureType[pTexture->type], pTexture->texture);
  glUniform1i((int)param, (int)textureUnit);
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
