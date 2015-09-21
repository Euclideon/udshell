#include "hal/driver.h"

#if UDRENDER_DRIVER == UDDRIVER_NULL

#include "hal/shader.h"
#include "hal/texture.h"

// ***************************************************************************************
// Author: Manu Evans, May 2015
udShader* udShader_CreateShaderFromFile(const char *pFilename, udShaderType type)
{
  return nullptr;
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
udShader* udShader_CreateShader(const char *pSource, size_t length, udShaderType type)
{
  return nullptr;
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
udShaderProgram* udShader_CreateShaderProgram(udShader *pVertexShader, udShader *pPixelShader)
{
  return nullptr;
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
size_t udShader_GetNumAttributes(udShaderProgram *pProgram)
{
  return 0;
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
const char *udShader_GetAttributeName(udShaderProgram *pProgram, size_t i)
{
  return "";
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
size_t udShader_GetAttributeType(udShaderProgram *pProgram, size_t i)
{
  return 0;
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
size_t udShader_GetNumUniforms(udShaderProgram *pProgram)
{
  return 0;
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
const char *udShader_GetUniformName(udShaderProgram *pProgram, size_t i)
{
  return "";
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
size_t udShader_GetUniformType(udShaderProgram *pProgram, size_t i)
{
  return 0;
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
int udShader_FindShaderParameter(udShaderProgram *pProgram, const char *pName)
{
  return -1;
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
void udShader_SetCurrent(udShaderProgram *pProgram)
{
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
void udShader_SetProgramData(int param, bool value)
{
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
void udShader_SetProgramData(int param, int value)
{
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
void udShader_SetProgramData(int param, float value)
{
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
void udShader_SetProgramData(int param, const udFloat4 &value)
{
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
void udShader_SetProgramData(int param, const udFloat4x4 &value)
{
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
void udShader_SetProgramData(int textureUnit, int param, struct udTexture *pTexture)
{
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
void udShader_SetProgramData(int param, const int *pValues, size_t count)
{
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
void udShader_SetProgramData(int param, const float *pValues, size_t count)
{
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
void udShader_SetProgramData(int param, const udFloat4 *pValues, size_t count)
{
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
void udShader_SetProgramData(int param, const udFloat4x4 *pValues, size_t count)
{
}

#endif // UDRENDER_DRIVER == UDDRIVER_OPENGL
