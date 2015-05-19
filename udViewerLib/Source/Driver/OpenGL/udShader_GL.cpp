#include "udViewerPlatform.h"

#if UDRENDER_DRIVER == UDDRIVER_OPENGL

#include "udShader.h"
#include "udTexture.h"
#include "udVertex.h"

#include "udOpenGL_Internal.h"


static GLenum s_shaderType[udST_Max] =
{
  GL_VERTEX_SHADER,
  GL_FRAGMENT_SHADER
};

extern GLenum s_textureType[udTT_Max];


// ***************************************************************************************
// Author: Manu Evans, May 2015
udShader* udShader_CreateShaderFromFile(const char *pFilename, udShaderType type)
{
  // load file
  //...
  GLchar *pSource = nullptr;

  return udShader_CreateShader(pSource, 0, type);
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
udShader* udShader_CreateShader(const char *pSource, size_t length, udShaderType type)
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

  udShader *pShader = udAllocType(udShader, 1, udAF_None);
  pShader->shader = shader;

  return pShader;
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
udShaderProgram* udShader_CreateShaderProgram(udShader *pVertexShader, udShader *pPixelShader)
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

  udShaderProgram *pProgram = udAllocType(udShaderProgram, 1, udAF_None);
  pProgram->program = program;

  // TODO: collect attribute and uniform id's

  return pProgram;
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
int udShader_FindShaderParameter(udShaderProgram *pProgram, const char *pName)
{
  return (int)glGetUniformLocation(pProgram->program, pName);
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
void udShader_SetCurrent(udShaderProgram *pProgram)
{
  if(pProgram)
    glUseProgram(pProgram->program);
  else
    glUseProgram(0);
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
void udShader_SetProgramData(int param, bool value)
{
  if(param < 0)
    return;
  glUniform1i(param, value ? 1 : 0);
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
void udShader_SetProgramData(int param, int value)
{
  if(param < 0)
    return;
  glUniform1i(param, value);
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
void udShader_SetProgramData(int param, float value)
{
  if(param < 0)
    return;
  glUniform1f(param, value);
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
void udShader_SetProgramData(int param, const udFloat4 &value)
{
  if(param < 0)
    return;
  glUniform4fv(param, 1, (GLfloat*)&value);
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
void udShader_SetProgramData(int param, const udFloat4x4 &value)
{
  if(param < 0)
    return;
  glUniformMatrix4fv(param, 1, 0, (GLfloat*)&value);
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
void udShader_SetProgramData(int textureUnit, int param, struct udTexture *pTexture)
{
  if(param < 0)
    return;
  glActiveTexture(GL_TEXTURE0 + textureUnit);
  glBindTexture(s_textureType[pTexture->type], pTexture->texture);
  glUniform1i(param, textureUnit);
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
void udShader_SetProgramData(int param, const int *pValues, size_t count)
{
  if(param < 0)
    return;
  glUniform1iv(param, (GLsizei)count, pValues);
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
void udShader_SetProgramData(int param, const float *pValues, size_t count)
{
  if(param < 0)
    return;
  glUniform1fv(param, (GLsizei)count, pValues);
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
void udShader_SetProgramData(int param, const udFloat4 *pValues, size_t count)
{
  if(param < 0)
    return;
  glUniform4fv(param, (GLsizei)count, (GLfloat*)pValues);
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
void udShader_SetProgramData(int param, const udFloat4x4 *pValues, size_t count)
{
  if(param < 0)
    return;
  glUniformMatrix4fv(param, (GLsizei)count, 0, (GLfloat*)pValues);
}

#endif // UDRENDER_DRIVER == UDDRIVER_OPENGL
