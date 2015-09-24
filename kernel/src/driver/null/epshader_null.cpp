#include "hal/driver.h"

#if EPRENDER_DRIVER == EPDRIVER_NULL

#include "hal/shader.h"
#include "hal/texture.h"

// ***************************************************************************************
// Author: Manu Evans, May 2015
epShader* epShader_CreateShaderFromFile(const char *pFilename, epShaderType type)
{
  return nullptr;
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
epShader* epShader_CreateShader(const char *pSource, size_t length, epShaderType type)
{
  return nullptr;
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
epShaderProgram* epShader_CreateShaderProgram(epShader *pVertexShader, epShader *pPixelShader)
{
  return nullptr;
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
size_t epShader_GetNumAttributes(epShaderProgram *pProgram)
{
  return 0;
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
const char *epShader_GetAttributeName(epShaderProgram *pProgram, size_t i)
{
  return "";
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
size_t epShader_GetAttributeType(epShaderProgram *pProgram, size_t i)
{
  return 0;
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
size_t epShader_GetNumUniforms(epShaderProgram *pProgram)
{
  return 0;
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
const char *epShader_GetUniformName(epShaderProgram *pProgram, size_t i)
{
  return "";
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
size_t epShader_GetUniformType(epShaderProgram *pProgram, size_t i)
{
  return 0;
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
int epShader_FindShaderParameter(epShaderProgram *pProgram, const char *pName)
{
  return -1;
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
void epShader_SetCurrent(epShaderProgram *pProgram)
{
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
void epShader_SetProgramData(int param, bool value)
{
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
void epShader_SetProgramData(int param, int value)
{
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
void epShader_SetProgramData(int param, float value)
{
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
void epShader_SetProgramData(int param, const udFloat4 &value)
{
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
void epShader_SetProgramData(int param, const udFloat4x4 &value)
{
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
void epShader_SetProgramData(int textureUnit, int param, struct epTexture *pTexture)
{
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
void epShader_SetProgramData(int param, const int *pValues, size_t count)
{
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
void epShader_SetProgramData(int param, const float *pValues, size_t count)
{
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
void epShader_SetProgramData(int param, const udFloat4 *pValues, size_t count)
{
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
void epShader_SetProgramData(int param, const udFloat4x4 *pValues, size_t count)
{
}

#endif // EPRENDER_DRIVER == EPDRIVER_OPENGL
