#include "hal/driver.h"

#if EPRENDER_DRIVER == EPDRIVER_QT

#include "hal/vertex.h"

#include "eprender_qt.h"

#include <QOpenGLBuffer>


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
epFormatDeclaration *epVertex_CreateFormatDeclaration(const epArrayElement *pElementArray, int elementCount)
{
  size_t size = sizeof(epFormatDeclaration) + (sizeof(epArrayElement) + sizeof(epArrayElementData))*elementCount;
  epFormatDeclaration *pDecl = (epFormatDeclaration*)epAlloc(size);
  pDecl->pElements = (epArrayElement*)&pDecl[1];
  pDecl->pElementData = (epArrayElementData*)(pDecl->pElements + elementCount);
  pDecl->numElements = elementCount;

  memcpy(pDecl->pElements, pElementArray, sizeof(epArrayElement)*elementCount);

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
void epVertex_DestroyFormatDeclaration(epFormatDeclaration **ppDeclaration)
{
  epFree(*ppDeclaration);
  *ppDeclaration = nullptr;
}

// ***************************************************************************************
epArrayBuffer* epVertex_CreateIndexBuffer(epArrayDataFormat format)
{
  bool result = true;
  epArrayBuffer *pIB = nullptr;
  QOpenGLBuffer *pQtBuffer = new QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
  EP_ERROR_IF(!pQtBuffer->create(), false);
  EP_ERROR_IF(!pQtBuffer->bind(), false);

  pIB = (epArrayBuffer*)epAlloc(sizeof(epArrayBuffer) + sizeof(epArrayDataFormat)*1);
  pIB->pFormat = (epArrayDataFormat*)&pIB[1];
  *pIB->pFormat = format;
  pIB->numElements = 1;
  pIB->pBuffer = pQtBuffer;

epilogue:
  if (!result)
  {
    epDebugPrintf("Error creating vertex buffer\n");
    delete pQtBuffer;
  }
  return pIB;
}

// ***************************************************************************************
epArrayBuffer* epVertex_CreateVertexBuffer(epArrayDataFormat elements[], size_t numElements)
{
  bool result = true;
  epArrayBuffer *pVB = nullptr;
  QOpenGLBuffer *pQtBuffer = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
  EP_ERROR_IF(!pQtBuffer->create(), false);
  EP_ERROR_IF(!pQtBuffer->bind(), false);

  pVB = (epArrayBuffer*)epAlloc(sizeof(epArrayBuffer) + sizeof(epArrayDataFormat)*numElements);
  pVB->pFormat = (epArrayDataFormat*)&pVB[1];
  memcpy(pVB->pFormat, elements, sizeof(epArrayDataFormat)*numElements);
  pVB->numElements = numElements;
  pVB->pBuffer = pQtBuffer;

epilogue:
  if (!result)
  {
    epDebugPrintf("Error creating vertex buffer\n");
    delete pQtBuffer;
  }
  return pVB;
}

// ***************************************************************************************
void epVertex_DestroyArrayBuffer(epArrayBuffer **ppBuffer)
{
  (*ppBuffer)->pBuffer->release();
  (*ppBuffer)->pBuffer->destroy();
  delete (*ppBuffer)->pBuffer;
  epFree(*ppBuffer);
  *ppBuffer = nullptr;
}

// ***************************************************************************************
void epVertex_SetArrayBufferData(epArrayBuffer *pBuffer, const void *pVertexData, size_t bufferLen)
{
  // TODO: remove these checks once we are confident in Kernel and the Qt driver
  EPASSERT(pBuffer->pBuffer != nullptr, "Qt buffer doesn't exist");

  pBuffer->pBuffer->bind();
  pBuffer->pBuffer->setUsagePattern(QOpenGLBuffer::StaticDraw);
  pBuffer->pBuffer->allocate(pVertexData, static_cast<int>(bufferLen));
}

#else
EPEMPTYFILE
#endif // EPRENDER_DRIVER == EPDRIVER_QT
