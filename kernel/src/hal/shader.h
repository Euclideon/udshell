#pragma once
#ifndef EPSHADER_H
#define EPSHADER_H

#include "ep/cpp/math.h"

struct epShader;
struct epShaderProgram;

enum epShaderType
{
  epST_VertexShader,
  epST_PixelShader,

  epST_Max
};

epShader* epShader_CreateShaderFromFile(const char *pFilename, epShaderType type);
epShader* epShader_CreateShader(const char *pSource, size_t length, epShaderType type);
void epShader_DestroyShader(epShader **ppShader);

epShaderProgram* epShader_CreateShaderProgram(epShader *pVertexShader, epShader *pPixelShader);
void epShader_DestroyShaderProgram(epShaderProgram **ppProgram);

size_t epShader_GetNumAttributes(epShaderProgram *pProgram);
const char *epShader_GetAttributeName(epShaderProgram *pProgram, size_t i);
size_t epShader_GetAttributeType(epShaderProgram *pProgram, size_t i);

size_t epShader_GetNumUniforms(epShaderProgram *pProgram);
const char *epShader_GetUniformName(epShaderProgram *pProgram, size_t i);
size_t epShader_GetUniformType(epShaderProgram *pProgram, size_t i);

int epShader_FindShaderParameter(epShaderProgram *pProgram, const char *pName);

void epShader_SetCurrent(epShaderProgram *pProgram);

void epShader_SetProgramData(int param, bool value);
void epShader_SetProgramData(int param, int value);
void epShader_SetProgramData(int param, float value);
void epShader_SetProgramData(int param, const ep::Float4 &value);
void epShader_SetProgramData(int param, const ep::Float4x4 &value);
void epShader_SetProgramData(int textureUnit, int param, struct epTexture *pTexture);

//void epShader_SetProgramData(epShaderProgram *pProgram, int param, const bool *pValues, size_t count);
void epShader_SetProgramData(int param, const int *pValues, size_t count);
void epShader_SetProgramData(int param, const float *pValues, size_t count);
void epShader_SetProgramData(int param, const ep::Float4 *pValues, size_t count);
void epShader_SetProgramData(int param, const ep::Float4x4 *pValues, size_t count);

#endif // EPSHADER_H
