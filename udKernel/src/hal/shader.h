#pragma once
#ifndef UDSHADER_H
#define UDSHADER_H

#include "udMath.h"

struct udShader;
struct udShaderProgram;

enum udShaderType
{
  udST_VertexShader,
  udST_PixelShader,

  udST_Max
};

udShader* udShader_CreateShaderFromFile(const char *pFilename, udShaderType type);
udShader* udShader_CreateShader(const char *pSource, size_t length, udShaderType type);

udShaderProgram* udShader_CreateShaderProgram(udShader *pVertexShader, udShader *pPixelShader);

int udShader_FindShaderParameter(udShaderProgram *pProgram, const char *pName);

void udShader_SetCurrent(udShaderProgram *pProgram);

void udShader_SetProgramData(int param, bool value);
void udShader_SetProgramData(int param, int value);
void udShader_SetProgramData(int param, float value);
void udShader_SetProgramData(int param, const udFloat4 &value);
void udShader_SetProgramData(int param, const udFloat4x4 &value);
void udShader_SetProgramData(int textureUnit, int param, struct udTexture *pTexture);

//void udShader_SetProgramData(udShaderProgram *pProgram, int param, const bool *pValues, size_t count);
void udShader_SetProgramData(int param, const int *pValues, size_t count);
void udShader_SetProgramData(int param, const float *pValues, size_t count);
void udShader_SetProgramData(int param, const udFloat4 *pValues, size_t count);
void udShader_SetProgramData(int param, const udFloat4x4 *pValues, size_t count);

#endif // UDSHADER_H
