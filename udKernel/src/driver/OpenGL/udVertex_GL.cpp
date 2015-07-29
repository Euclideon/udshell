#include "hal/driver.h"

#if UDRENDER_DRIVER == UDDRIVER_OPENGL

#include "hal/vertex.h"

#include "udOpenGL_Internal.h"


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
// Author: Manu Evans, May 2015
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
// Author: Manu Evans, May 2015
void udVertex_DestroyVertexDeclaration(udVertexDeclaration **ppDeclaration)
{
  udFree(*ppDeclaration);
  *ppDeclaration = nullptr;
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
udVertexBuffer* udVertex_CreateVertexBuffer(udVertexDeclaration *pFormat)
{
  udVertexBuffer *pVB = udAllocType(udVertexBuffer, 1, udAF_Zero);

  pVB->pVertexDeclaration = pFormat;

  glGenBuffers(1, &pVB->vb);
  glBindBuffer(GL_ARRAY_BUFFER, pVB->vb);

  return pVB;
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
void udVertex_DestroyVertexBuffer(udVertexBuffer **ppVB)
{
  glDeleteBuffers(1, &(*ppVB)->vb);
  udFree(*ppVB);
  *ppVB = nullptr;
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
void udVertex_SetVertexBufferData(udVertexBuffer *pVB, void *pVertexData, size_t bufferLen)
{
  glBindBuffer(GL_ARRAY_BUFFER, pVB->vb);
  glBufferData(GL_ARRAY_BUFFER, bufferLen, pVertexData, GL_STATIC_DRAW);

  pVB->bufferLen = bufferLen;
}


#endif // UDRENDER_DRIVER == UDDRIVER_OPENGL
