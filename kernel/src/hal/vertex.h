#pragma once
#ifndef UDVERTEX_H
#define UDVERTEX_H

struct udFormatDeclaration;
struct udArrayBuffer;

enum udArrayDataFormat
{
  udVDF_Unknown = -1,

  udVDF_Float4 = 0,
  udVDF_Float3,
  udVDF_Float2,
  udVDF_Float,
  udVDF_UByte4N_RGBA,
  udVDF_UByte4N_BGRA,
  udVDF_Int4,
  udVDF_Int3,
  udVDF_Int2,
  udVDF_Int,
  udVDF_UInt4,
  udVDF_UInt3,
  udVDF_UInt2,
  udVDF_UInt,
  udVDF_Short4,
  udVDF_Short2,
  udVDF_Short4N,
  udVDF_Short2N,
  udVDF_Short,
  udVDF_UShort4,
  udVDF_UShort2,
  udVDF_UShort4N,
  udVDF_UShort2N,
  udVDF_UShort,
  udVDF_Byte4,
  udVDF_UByte4,
  udVDF_Byte4N,
  udVDF_Byte,
  udVDF_UByte,

  udVDF_Max,
  udVDF_ForceInt = 0x7f
};

struct udArrayElement
{
  char attributeName[56];
  udArrayDataFormat format;
//  int componentCount;
  int stream;
};

udFormatDeclaration *udVertex_CreateFormatDeclaration(const udArrayElement *pElementArray, int elementCount);
void udVertex_DestroyFormatDeclaration(udFormatDeclaration **ppDeclaration);

udArrayBuffer* udVertex_CreateIndexBuffer(udArrayDataFormat format);
udArrayBuffer* udVertex_CreateVertexBuffer(udArrayDataFormat elements[], size_t numElements);
void udVertex_DestroyArrayBuffer(udArrayBuffer **ppVB);

void udVertex_SetArrayBufferData(udArrayBuffer *pVB, const void *pData, size_t bufferLen);

//void udVertex_SetVertexBuffer(const udArrayBuffer *pVertexBuffer);
//void udVertex_SetIndexBuffer(const udIndexBuffer *pIndexBuffer);

#endif // UDVERTEX_H
