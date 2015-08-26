#include "hal/driver.h"

#if UDRENDER_DRIVER == UDDRIVER_OPENGL

#include "hal/vertex.h"

#include "udOpenGL_Internal.h"


const int s_VertexDataStride[udVDF_Max] =
{
  16,	// udVDF_Float4
  12,	// udVDF_Float3
  8,	// udVDF_Float2
  4,	// udVDF_Float
  4,	// udVDF_UByte4N_RGBA
  4,	// udVDF_UByte4N_BGRA
  16, // udVDF_Int4
  12, // udVDF_Int3
  8,  // udVDF_Int2
  4,  // udVDF_Int
  16, // udVDF_UInt4
  12, // udVDF_UInt3
  8,  // udVDF_UInt2
  4,  // udVDF_UInt
  8,  // udVDF_Short4
  4,  // udVDF_Short2
  8,  // udVDF_Short4N
  4,  // udVDF_Short2N
  2,  // udVDF_Short
  8,  // udVDF_UShort4
  4,  // udVDF_UShort2
  8,  // udVDF_UShort4N
  4,  // udVDF_UShort2N
  2,  // udVDF_UShort
  4,  // udVDF_Byte4
  4,  // udVDF_UByte4
  4,  // udVDF_Byte4N
  1,  // udVDF_Byte
  1,  // udVDF_UByte
};


// ***************************************************************************************
// Author: Manu Evans, May 2015
udFormatDeclaration *udVertex_CreateFormatDeclaration(const udArrayElement *pElementArray, int elementCount)
{
  size_t size = sizeof(udFormatDeclaration) + (sizeof(udArrayElement) + sizeof(udArrayElementData))*elementCount;
  udFormatDeclaration *pDecl = (udFormatDeclaration*)udAlloc(size);
  pDecl->pElements = (udArrayElement*)&pDecl[1];
  pDecl->pElementData = (udArrayElementData*)(pDecl->pElements + elementCount);
  pDecl->numElements = elementCount;

  memcpy(pDecl->pElements, pElementArray, sizeof(udArrayElement)*elementCount);

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
void udVertex_DestroyFormatDeclaration(udFormatDeclaration **ppDeclaration)
{
  udFree(*ppDeclaration);
  *ppDeclaration = nullptr;
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
udArrayBuffer* udVertex_CreateIndexBuffer(udArrayDataFormat format)
{
  udArrayBuffer *pIB = (udArrayBuffer*)udAlloc(sizeof(udArrayBuffer) + sizeof(udArrayDataFormat)*1);
  pIB->pFormat = (udArrayDataFormat*)&(pIB[1]);
  pIB->type = udAT_IndexArray;
  *pIB->pFormat = format;
  pIB->numElements = 1;

  glGenBuffers(1, &pIB->buffer);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pIB->buffer);

  return pIB;
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
udArrayBuffer* udVertex_CreateVertexBuffer(udArrayDataFormat elements[], size_t numElements)
{
  udArrayBuffer *pVB = (udArrayBuffer*)udAlloc(sizeof(udArrayBuffer) + sizeof(udArrayDataFormat)*numElements);
  pVB->pFormat = (udArrayDataFormat*)&(pVB[1]);
  pVB->type = udAT_VertexArray;
  memcpy(pVB->pFormat, elements, sizeof(udArrayDataFormat)*numElements);
  pVB->numElements = numElements;

  glGenBuffers(1, &pVB->buffer);
  glBindBuffer(GL_ARRAY_BUFFER, pVB->buffer);

  return pVB;
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
void udVertex_DestroyArrayBuffer(udArrayBuffer **ppBuffer)
{
  glDeleteBuffers(1, &(*ppBuffer)->buffer);
  udFree(*ppBuffer);
  *ppBuffer = nullptr;
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
void udVertex_SetArrayBufferData(udArrayBuffer *pBuffer, const void *pVertexData, size_t bufferLen)
{
  GLenum type = pBuffer->type == udAT_IndexArray ? GL_ELEMENT_ARRAY_BUFFER : GL_ARRAY_BUFFER;
  glBindBuffer(type, pBuffer->buffer);
  glBufferData(type, bufferLen, pVertexData, GL_STATIC_DRAW);

  pBuffer->bufferLen = bufferLen;
}


#endif // UDRENDER_DRIVER == UDDRIVER_OPENGL
