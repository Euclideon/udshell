#pragma once
#ifndef UDVERTEX_H
#define UDVERTEX_H

struct udVertexDeclaration;
struct udVertexBuffer;

enum udVertexDataFormat
{
  udVDF_Float4,
  udVDF_Float3,
  udVDF_Float2,
  udVDF_Float1,
  udVDF_UByte4N_RGBA,
  udVDF_UByte4N_BGRA,

  udVDF_Max
};

enum udVertexElementType
{
  udVET_Position,
  udVET_Normal,
  udVET_Colour,
  udVET_TexCoord,

  udVET_Max
};

struct udVertexElement
{
  udVertexElementType type;
  int index;
  int componentCount;
  udVertexDataFormat format;
};

udVertexDeclaration *udVertex_CreateVertexDeclaration(const udVertexElement *pElementArray, int elementCount);
void udVertex_DestroyVertexDeclaration(udVertexDeclaration **ppDeclaration);

udVertexBuffer* udVertex_CreateVertexBuffer(udVertexDeclaration *pFormat);
void udVertex_DestroyVertexBuffer(udVertexBuffer **ppVB);

void udVertex_SetVertexBufferData(udVertexBuffer *pVB, void *pVertexData, size_t bufferLen);

//void udVertex_SetVertexBuffer(const udVertexBuffer *pVertexBuffer);
//void udVertex_SetIndexBuffer(const udIndexBuffer *pIndexBuffer);

#endif // UDVERTEX_H
