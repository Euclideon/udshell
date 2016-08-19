#pragma once
#ifndef EPSHADER_H
#define EPSHADER_H

#include "ep/cpp/math.h"

struct epShader;
struct epShaderProgram;

enum epShaderType
{
  epST_VertexShader = 0,
  epST_PixelShader,
  epST_GeometryShader,
  epST_TesselationControl,
  epST_TesselationEvaluation,
  epST_Compute,

  epST_Max
};

enum epShaderElementType
{
  epSET_Int = 0,
  epSET_Uint,
  epSET_Float,
  epSET_Double
};

enum epShaderSamplerType
{
  epSST_None,
  epSST_Default,
  epSST_Shadow,
  epSST_Rect,
  epSST_ShadowRect,
  epSST_Multisample,
  epSST_Buffer // TODO: is this a sampler 'type', or a 'dimensions'?
};

enum epShaderSamplerDimensions
{
  epSSD_1D,
  epSSD_1DArray,
  epSSD_2D,
  epSSD_2DArray,
  epSSD_Cube,
  epSSD_CubeArray,
  epSSD_3D,
  epSSD_Buffer // TODO: is this a sampler 'type', or a 'dimensions'?
};

struct epShaderElement
{
  uint64_t n : 3; // n > 1 => element is vector
  uint64_t m : 3; // m > 1 => element is matrix
  uint64_t type : 3;
  uint64_t samplerType : 3;
  uint64_t samplerDimensions : 3;
  uint64_t arrayLength : 24;
  uint64_t location : 16; // opengls max attribute and uniform values are within 16 bits
  uint64_t reserved : 9;
};

epShader* epShader_CreateShaderFromFile(const char *pFilename, epShaderType type);
epShader* epShader_CreateShader(const char *pSource, size_t length, epShaderType type);
void epShader_DestroyShader(epShader **ppShader);

epShaderProgram* epShader_CreateShaderProgram(epShader *shaders[], size_t arrayLength);
void epShader_DestroyShaderProgram(epShaderProgram **ppProgram);

size_t epShader_GetNumAttributes(epShaderProgram *pProgram);
const char *epShader_GetAttributeName(epShaderProgram *pProgram, size_t i);
epShaderElement epShader_GetAttributeType(epShaderProgram *pProgram, size_t i);

size_t epShader_GetNumUniforms(epShaderProgram *pProgram);
const char *epShader_GetUniformName(epShaderProgram *pProgram, size_t i);
epShaderElement epShader_GetUniformType(epShaderProgram *pProgram, size_t i);

void epShader_GetElementTypeString(epShaderElement t, char *pBuffer, size_t size, size_t *pLength);

void epShader_GetProgramData(epShaderProgram *pProgram, size_t param, bool *pValue);
void epShader_GetProgramData(epShaderProgram *pProgram, size_t param, int *pValue);
void epShader_GetProgramData(epShaderProgram *pProgram, size_t param, uint32_t *pValue);
void epShader_GetProgramData(epShaderProgram *pProgram, size_t param, float *pValue);
void epShader_GetProgramData(epShaderProgram *pProgram, size_t param, double *pValue);

void epShader_GetProgramData(epShaderProgram *pProgram, size_t param, ep::Vector2<int> *pValue);
void epShader_GetProgramData(epShaderProgram *pProgram, size_t param, ep::Vector3<int> *pValue);
void epShader_GetProgramData(epShaderProgram *pProgram, size_t param, ep::Vector4<int> *pValue);

void epShader_GetProgramData(epShaderProgram *pProgram, size_t param, ep::Vector2<uint32_t> *pValue);
void epShader_GetProgramData(epShaderProgram *pProgram, size_t param, ep::Vector3<uint32_t> *pValue);
void epShader_GetProgramData(epShaderProgram *pProgram, size_t param, ep::Vector4<uint32_t> *pValue);

void epShader_GetProgramData(epShaderProgram *pProgram, size_t param, ep::Float2 *pValue);
void epShader_GetProgramData(epShaderProgram *pProgram, size_t param, ep::Float3 *pValue);
void epShader_GetProgramData(epShaderProgram *pProgram, size_t param, ep::Float4 *pValue);
void epShader_GetProgramData(epShaderProgram *pProgram, size_t param, ep::Float4x4 *pValue);

void epShader_GetProgramData(epShaderProgram *pProgram, size_t param, ep::Float2 *pValue);
void epShader_GetProgramData(epShaderProgram *pProgram, size_t param, ep::Float3 *pValue);
void epShader_GetProgramData(epShaderProgram *pProgram, size_t param, ep::Float4 *pValue);
void epShader_GetProgramData(epShaderProgram *pProgram, size_t param, ep::Float4x4 *pValue);

void epShader_GetProgramData(epShaderProgram *pProgram, size_t param, ep::Double2 *pValue);
void epShader_GetProgramData(epShaderProgram *pProgram, size_t param, ep::Double3 *pValue);
void epShader_GetProgramData(epShaderProgram *pProgram, size_t param, ep::Double4 *pValue);
void epShader_GetProgramData(epShaderProgram *pProgram, size_t param, ep::Double4x4 *pValue);

// The following 3 functions are for use with parameters that aren't part of our math lib
void epShader_GetProgramData(epShaderProgram *pProgram, size_t param, int *pValue, size_t);
void epShader_GetProgramData(epShaderProgram *pProgram, size_t param, float *pValue, size_t);
void epShader_GetProgramData(epShaderProgram *pProgram, size_t param, double *pValue, size_t);


int epShader_FindShaderParameter(epShaderProgram *pProgram, const char *pName);
int epShader_FindShaderAttribute(epShaderProgram *pProgram, const char *pName);

void epShader_SetCurrent(epShaderProgram *pProgram);


void epShader_SetProgramData(size_t param, bool value);

void epShader_SetProgramData(size_t param, const int *value, size_t count);
void epShader_SetProgramData(size_t param, const ep::Vector2<int> *value, size_t count);
void epShader_SetProgramData(size_t param, const ep::Vector3<int> *value, size_t count);
void epShader_SetProgramData(size_t param, const ep::Vector4<int> *value, size_t count);

void epShader_SetProgramData(size_t param, const uint32_t *value, size_t count);
void epShader_SetProgramData(size_t param, const ep::Vector2<uint32_t> *value, size_t count);
void epShader_SetProgramData(size_t param, const ep::Vector3<uint32_t> *value, size_t count);
void epShader_SetProgramData(size_t param, const ep::Vector4<uint32_t> *value, size_t count);

void epShader_SetProgramData(size_t param, const float *value, size_t count);
void epShader_SetProgramData(size_t param, const ep::Vector2<float> *value, size_t count);
void epShader_SetProgramData(size_t param, const ep::Vector3<float> *value, size_t count);
void epShader_SetProgramData(size_t param, const ep::Vector4<float> *value, size_t count);
void epShader_SetProgramData(size_t param, const ep::Matrix4x4<float> *value, size_t count);

void epShader_SetProgramData(size_t param, const double *value, size_t count);
void epShader_SetProgramData(size_t param, const ep::Vector2<double> *value, size_t count);
void epShader_SetProgramData(size_t param, const ep::Vector3<double> *value, size_t count);
void epShader_SetProgramData(size_t param, const ep::Vector4<double> *value, size_t count);
void epShader_SetProgramData(size_t param, const ep::Matrix4x4<double> *value, size_t count);

void epShader_SetProgramData(size_t textureUnit, size_t param, struct epTexture *pTexture);

// TODO: sampler arrays?

// inlines for single element
inline void epShader_SetProgramData(size_t param, int value) { epShader_SetProgramData(param, &value, 1); }
inline void epShader_SetProgramData(size_t param, const ep::Vector2<int> &value) { epShader_SetProgramData(param, &value, 1); }
inline void epShader_SetProgramData(size_t param, const ep::Vector3<int> &value) { epShader_SetProgramData(param, &value, 1); }
inline void epShader_SetProgramData(size_t param, const ep::Vector4<int> &value) { epShader_SetProgramData(param, &value, 1); }

inline void epShader_SetProgramData(size_t param, uint32_t value) { epShader_SetProgramData(param, &value, 1); }
inline void epShader_SetProgramData(size_t param, const ep::Vector2<uint32_t> &value) { epShader_SetProgramData(param, &value, 1); }
inline void epShader_SetProgramData(size_t param, const ep::Vector3<uint32_t> &value) { epShader_SetProgramData(param, &value, 1); }
inline void epShader_SetProgramData(size_t param, const ep::Vector4<uint32_t> &value) { epShader_SetProgramData(param, &value, 1); }

inline void epShader_SetProgramData(size_t param, float value) { epShader_SetProgramData(param, &value, 1); }
inline void epShader_SetProgramData(size_t param, const ep::Vector2<float> &value) { epShader_SetProgramData(param, &value, 1); }
inline void epShader_SetProgramData(size_t param, const ep::Vector3<float> &value) { epShader_SetProgramData(param, &value, 1); }
inline void epShader_SetProgramData(size_t param, const ep::Vector4<float> &value) { epShader_SetProgramData(param, &value, 1); }
inline void epShader_SetProgramData(size_t param, const ep::Matrix4x4<float> &value) { epShader_SetProgramData(param, &value, 1); }

inline void epShader_SetProgramData(size_t param, double value) { epShader_SetProgramData(param, &value, 1); }
inline void epShader_SetProgramData(size_t param, const ep::Vector2<double> &value) { epShader_SetProgramData(param, &value, 1); }
inline void epShader_SetProgramData(size_t param, const ep::Vector3<double> &value) { epShader_SetProgramData(param, &value, 1); }
inline void epShader_SetProgramData(size_t param, const ep::Vector4<double> &value) { epShader_SetProgramData(param, &value, 1); }
inline void epShader_SetProgramData(size_t param, const ep::Matrix4x4<double> &value) { epShader_SetProgramData(param, &value, 1); }

#endif // EPSHADER_H
