#include "hal/driver.h"

#if EPRENDER_DRIVER == EPDRIVER_OPENGL

#include "hal/vertex.h"

#include "ep_opengl.h"


const int s_VertexDataStride[epVDF_Max] =
{
  16,	// epVDF_Float4
  12,	// epVDF_Float3
  8,	// epVDF_Float2
  4,	// epVDF_Float
  4,	// epVDF_UByte4N_RGBA
  4,	// epVDF_UByte4N_BGRA
  16, // epVDF_Int4
  12, // epVDF_Int3
  8,  // epVDF_Int2
  4,  // epVDF_Int
  16, // epVDF_UInt4
  12, // epVDF_UInt3
  8,  // epVDF_UInt2
  4,  // epVDF_UInt
  8,  // epVDF_Short4
  4,  // epVDF_Short2
  8,  // epVDF_Short4N
  4,  // epVDF_Short2N
  2,  // epVDF_Short
  8,  // epVDF_UShort4
  4,  // epVDF_UShort2
  8,  // epVDF_UShort4N
  4,  // epVDF_UShort2N
  2,  // epVDF_UShort
  4,  // epVDF_Byte4
  4,  // epVDF_UByte4
  4,  // epVDF_Byte4N
  1,  // epVDF_Byte
  1,  // epVDF_UByte
};


// ***************************************************************************************
// Author: Manu Evans, May 2015
epFormatDeclaration *epVertex_CreateFormatDeclaration(const epArrayElement *pElementArray, int elementCount)
{
  size_t size = sizeof(epFormatDeclaration) + (sizeof(epArrayElement) + sizeof(epArrayElementData))*elementCount;
  epFormatDeclaration *pDecl = (epFormatDeclaration*)epAlloc(size);
  pDecl->pElements = (epArrayElement*)&pDecl[1];
  pDecl->pElementData = (epArrayElementData*)(pDecl->pElements + elementCount);
  pDecl->numElements = elementCount;

  memcpy(pDecl->pElements, pElementArray, sizeof(epArrayElement)*elementCount);

  // set the element data and calculate the strides
  int streamOffset = 0;
  for (int e=0; e<elementCount; ++e)
  {
    pDecl->pElementData[e].offset = streamOffset;
    streamOffset += s_VertexDataStride[pElementArray[e].format];
  }
  // set the strides for each component
  for (int e=0; e<elementCount; ++e)
    pDecl->pElementData[e].stride = streamOffset;

  return pDecl;
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
void epVertex_DestroyFormatDeclaration(epFormatDeclaration **ppDeclaration)
{
  epFree(*ppDeclaration);
  *ppDeclaration = nullptr;
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
epArrayBuffer* epVertex_CreateIndexBuffer(epArrayDataFormat format)
{
  epArrayBuffer *pIB = (epArrayBuffer*)epAlloc(sizeof(epArrayBuffer) + sizeof(epArrayDataFormat)*1);
  pIB->pFormat = (epArrayDataFormat*)&(pIB[1]);
  pIB->type = epAT_IndexArray;
  *pIB->pFormat = format;
  pIB->numElements = 1;

  glGenBuffers(1, &pIB->buffer);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pIB->buffer);

  return pIB;
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
epArrayBuffer* epVertex_CreateVertexBuffer(epArrayDataFormat elements[], size_t numElements)
{
  epArrayBuffer *pVB = (epArrayBuffer*)epAlloc(sizeof(epArrayBuffer) + sizeof(epArrayDataFormat)*numElements);
  pVB->pFormat = (epArrayDataFormat*)&(pVB[1]);
  pVB->type = epAT_VertexArray;
  memcpy(pVB->pFormat, elements, sizeof(epArrayDataFormat)*numElements);
  pVB->numElements = numElements;

  glGenBuffers(1, &pVB->buffer);
  glBindBuffer(GL_ARRAY_BUFFER, pVB->buffer);

  return pVB;
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
void epVertex_DestroyArrayBuffer(epArrayBuffer **ppBuffer)
{
  glDeleteBuffers(1, &(*ppBuffer)->buffer);
  epFree(*ppBuffer);
  *ppBuffer = nullptr;
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
void epVertex_SetArrayBufferData(epArrayBuffer *pBuffer, const void *pVertexData, size_t bufferLen)
{
  GLenum type = pBuffer->type == epAT_IndexArray ? GL_ELEMENT_ARRAY_BUFFER : GL_ARRAY_BUFFER;
  glBindBuffer(type, pBuffer->buffer);
  glBufferData(type, bufferLen, pVertexData, GL_STATIC_DRAW);

  pBuffer->bufferLen = bufferLen;
}


#endif // EPRENDER_DRIVER == EPDRIVER_OPENGL
