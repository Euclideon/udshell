#include "udDriver.h"

#if UDRENDER_DRIVER == UDDRIVER_QT

#include "udShader.h"
#include "udTexture.h"
#include "udVertex.h"


// ***************************************************************************************
udShader* udShader_CreateShaderFromFile(const char *pFilename, udShaderType type)
{
  return 0;
}

// ***************************************************************************************
udShader* udShader_CreateShader(const char *pSource, size_t length, udShaderType type)
{
  return 0;
}

// ***************************************************************************************
udShaderProgram* udShader_CreateShaderProgram(udShader *pVertexShader, udShader *pPixelShader)
{
  return 0;
}

// ***************************************************************************************
int udShader_FindShaderParameter(udShaderProgram *pProgram, const char *pName)
{
  return 0;
}

// ***************************************************************************************
void udShader_SetCurrent(udShaderProgram *pProgram)
{
}

// ***************************************************************************************
void udShader_SetProgramData(int param, bool value)
{
}

// ***************************************************************************************
void udShader_SetProgramData(int param, int value)
{
}

// ***************************************************************************************
void udShader_SetProgramData(int param, float value)
{
}

// ***************************************************************************************
void udShader_SetProgramData(int param, const udFloat4 &value)
{
}

// ***************************************************************************************
void udShader_SetProgramData(int param, const udFloat4x4 &value)
{
}

// ***************************************************************************************
void udShader_SetProgramData(int textureUnit, int param, struct udTexture *pTexture)
{
}

// ***************************************************************************************
void udShader_SetProgramData(int param, const int *pValues, size_t count)
{
}

// ***************************************************************************************
void udShader_SetProgramData(int param, const float *pValues, size_t count)
{
}

// ***************************************************************************************
void udShader_SetProgramData(int param, const udFloat4 *pValues, size_t count)
{
}

// ***************************************************************************************
void udShader_SetProgramData(int param, const udFloat4x4 *pValues, size_t count)
{
}

#endif // UDRENDER_DRIVER == UDDRIVER_QT
