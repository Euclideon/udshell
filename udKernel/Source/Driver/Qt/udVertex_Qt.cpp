#include "udDriver.h"

#if UDRENDER_DRIVER == UDDRIVER_QT

#include "udVertex.h"
#include "udQt_Internal.h"

#include <QOpenGLBuffer>


const int s_VertexDataStride[udVDF_Max] =
{
  16,	// udVDF_Float4
  12,	// udVDF_Float3
  8,	// udVDF_Float2
  4,	// udVDF_Float1
  4,	// udVDF_UByte4N_RGBA
  4,	// udVDF_UByte4N_BGRA
};


// ***************************************************************************************
udVertexDeclaration *udVertex_CreateVertexDeclaration(const udVertexElement *pElementArray, int elementCount)
{
  size_t size = sizeof(udVertexDeclaration) + (sizeof(udVertexElement) + sizeof(udVertexElementData))*elementCount;
  udVertexDeclaration *pDecl = (udVertexDeclaration*)udAlloc(size);
  pDecl->pElements = (udVertexElement*)&pDecl[1];
  pDecl->pElementData = (udVertexElementData*)(pDecl->pElements + elementCount);
  pDecl->numElements = elementCount;

  memcpy(pDecl->pElements, pElementArray, sizeof(udVertexElement)*elementCount);

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
void udVertex_DestroyVertexDeclaration(udVertexDeclaration **ppDeclaration)
{
  udFree(*ppDeclaration);
  *ppDeclaration = nullptr;
}

// ***************************************************************************************
udVertexBuffer* udVertex_CreateVertexBuffer(udVertexDeclaration *pFormat)
{
  bool result = true;
  udVertexBuffer *pVB = nullptr;
  QOpenGLBuffer *pQtBuffer = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
  UD_ERROR_IF(!pQtBuffer->create(), false);
  UD_ERROR_IF(!pQtBuffer->bind(), false);

  pVB = udAllocType(udVertexBuffer, 1, udAF_Zero);
  pVB->pVertexDeclaration = pFormat;
  pVB->pVB = pQtBuffer;

epilogue:
  if (!result)
  {
    udDebugPrintf("Error creating vertex buffer\n");
    delete pQtBuffer;
  }
  return pVB;
}

// ***************************************************************************************
void udVertex_DestroyVertexBuffer(udVertexBuffer **ppVB)
{
  UDASSERT(false, "TODO");
}

// ***************************************************************************************
void udVertex_SetVertexBufferData(udVertexBuffer *pVB, void *pVertexData, size_t bufferLen)
{
  // TODO: remove these checks once we are confident in udKernel and the Qt driver
  UDASSERT(pVB->pVB != nullptr, "Qt Vertex buffer doesn't exist");

  pVB->pVB->bind();
  pVB->pVB->setUsagePattern(QOpenGLBuffer::StaticDraw);
  pVB->pVB->allocate(pVertexData, static_cast<int>(bufferLen));
}


#endif // UDRENDER_DRIVER == UDDRIVER_QT
