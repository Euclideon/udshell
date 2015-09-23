#include "hal/driver.h"

#if UDRENDER_DRIVER == UDDRIVER_QT

#include "hal/vertex.h"

#include "eprender_qt.h"

#include <QOpenGLBuffer>


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
udFormatDeclaration *udVertex_CreateFormatDeclaration(const udArrayElement *pElementArray, int elementCount)
{
  size_t size = sizeof(udFormatDeclaration) + (sizeof(udArrayElement) + sizeof(udArrayElementData))*elementCount;
  udFormatDeclaration *pDecl = (udFormatDeclaration*)udAlloc(size);
  pDecl->pElements = (udArrayElement*)&pDecl[1];
  pDecl->pElementData = (udArrayElementData*)(pDecl->pElements + elementCount);
  pDecl->numElements = elementCount;

  memcpy(pDecl->pElements, pElementArray, sizeof(udArrayElement)*elementCount);

  // set the element data and calculate the strides
  int streamOffset[64] = { 0 };
  for (int e=0; e<elementCount; ++e)
  {
    pDecl->pElementData[e].offset = streamOffset[pElementArray[e].stream];
    streamOffset[pElementArray[e].stream] += s_VertexDataStride[pElementArray[e].format];
  }
  // set the strides for each component
  for (int e=0; e<elementCount; ++e)
    pDecl->pElementData[e].stride = streamOffset[pElementArray[e].stream];

  return pDecl;
}

// ***************************************************************************************
void udVertex_DestroyFormatDeclaration(udFormatDeclaration **ppDeclaration)
{
  udFree(*ppDeclaration);
  *ppDeclaration = nullptr;
}

// ***************************************************************************************
udArrayBuffer* udVertex_CreateIndexBuffer(udArrayDataFormat format)
{
  bool result = true;
  udArrayBuffer *pIB = nullptr;
  QOpenGLBuffer *pQtBuffer = new QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
  UD_ERROR_IF(!pQtBuffer->create(), false);
  UD_ERROR_IF(!pQtBuffer->bind(), false);

  pIB = (udArrayBuffer*)udAlloc(sizeof(udArrayBuffer) + sizeof(udArrayDataFormat)*1);
  pIB->pFormat = (udArrayDataFormat*)&pIB[1];
  *pIB->pFormat = format;
  pIB->numElements = 1;
  pIB->pBuffer = pQtBuffer;

epilogue:
  if (!result)
  {
    udDebugPrintf("Error creating vertex buffer\n");
    delete pQtBuffer;
  }
  return pIB;
}

// ***************************************************************************************
udArrayBuffer* udVertex_CreateVertexBuffer(udArrayDataFormat elements[], size_t numElements)
{
  bool result = true;
  udArrayBuffer *pVB = nullptr;
  QOpenGLBuffer *pQtBuffer = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
  UD_ERROR_IF(!pQtBuffer->create(), false);
  UD_ERROR_IF(!pQtBuffer->bind(), false);

  pVB = (udArrayBuffer*)udAlloc(sizeof(udArrayBuffer) + sizeof(udArrayDataFormat)*numElements);
  pVB->pFormat = (udArrayDataFormat*)&pVB[1];
  memcpy(pVB->pFormat, elements, sizeof(udArrayDataFormat)*numElements);
  pVB->numElements = numElements;
  pVB->pBuffer = pQtBuffer;

epilogue:
  if (!result)
  {
    udDebugPrintf("Error creating vertex buffer\n");
    delete pQtBuffer;
  }
  return pVB;
}

// ***************************************************************************************
void udVertex_DestroyArrayBuffer(udArrayBuffer **ppBuffer)
{
  (*ppBuffer)->pBuffer->release();
  (*ppBuffer)->pBuffer->destroy();
  delete (*ppBuffer)->pBuffer;
  udFree(*ppBuffer);
  *ppBuffer = nullptr;
}

// ***************************************************************************************
void udVertex_SetArrayBufferData(udArrayBuffer *pBuffer, const void *pVertexData, size_t bufferLen)
{
  // TODO: remove these checks once we are confident in Kernel and the Qt driver
  UDASSERT(pBuffer->pBuffer != nullptr, "Qt buffer doesn't exist");

  pBuffer->pBuffer->bind();
  pBuffer->pBuffer->setUsagePattern(QOpenGLBuffer::StaticDraw);
  pBuffer->pBuffer->allocate(pVertexData, static_cast<int>(bufferLen));
}


#endif // UDRENDER_DRIVER == UDDRIVER_QT
