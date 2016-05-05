#pragma once
#ifndef EPVERTEX_H
#define EPVERTEX_H

struct epFormatDeclaration;
struct epShaderProgram;
struct epArrayBuffer;

enum epArrayDataFormat
{
  epVDF_Unknown = -1,

  epVDF_Float4 = 0,
  epVDF_Float3,
  epVDF_Float2,
  epVDF_Float,
  epVDF_UByte4N_RGBA,
  epVDF_UByte4N_BGRA,
  epVDF_Int4,
  epVDF_Int3,
  epVDF_Int2,
  epVDF_Int,
  epVDF_UInt4,
  epVDF_UInt3,
  epVDF_UInt2,
  epVDF_UInt,
  epVDF_Short4,
  epVDF_Short2,
  epVDF_Short4N,
  epVDF_Short2N,
  epVDF_Short,
  epVDF_UShort4,
  epVDF_UShort2,
  epVDF_UShort4N,
  epVDF_UShort2N,
  epVDF_UShort,
  epVDF_Byte4,
  epVDF_UByte4,
  epVDF_Byte4N,
  epVDF_Byte,
  epVDF_UByte,

  epVDF_Max,
  epVDF_ForceInt = 0x7f
};

struct epArrayElement
{
  char attributeName[48];
  epArrayDataFormat format;
  int stream;
  int offset;
  int stride;
};

struct epShaderInputConfig;

epShaderInputConfig *epVertex_CreateShaderInputConfig(const epArrayElement *pElementArray, size_t elementCount, epShaderProgram *pShaderProgram);
void epVertex_GetShaderInputConfigStreams(const epShaderInputConfig *pConfig, int *pStreams, size_t streamsLength, int *pNumStreams);

void epVertex_DestroyShaderInputConfig(epShaderInputConfig **ppDeclaration);

epArrayBuffer* epVertex_CreateIndexBuffer(epArrayDataFormat format);
epArrayBuffer* epVertex_CreateVertexBuffer(epArrayDataFormat elements[], size_t numElements);
void epVertex_DestroyArrayBuffer(epArrayBuffer **ppVB);

void epVertex_SetArrayBufferData(epArrayBuffer *pVB, const void *pData, size_t bufferLen);

//void epVertex_SetVertexBuffer(const epArrayBuffer *pVertexBuffer);
//void epVertex_SetIndexBuffer(const udIndexBuffer *pIndexBuffer);

#endif // EPVERTEX_H
