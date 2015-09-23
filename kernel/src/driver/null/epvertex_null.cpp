#include "hal/driver.h"

#if UDRENDER_DRIVER == UDDRIVER_NULL

#include "hal/vertex.h"

// ***************************************************************************************
// Author: Manu Evans, May 2015
udFormatDeclaration *udVertex_CreateFormatDeclaration(const udArrayElement *pElementArray, int elementCount)
{
  return nullptr;
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
void udVertex_DestroyFormatDeclaration(udFormatDeclaration **ppDeclaration)
{
}

// ***************************************************************************************
// Author: Manu Evans, Aug 2015
udArrayBuffer* udVertex_CreateIndexBuffer(udArrayDataFormat format)
{
  return nullptr;
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
udArrayBuffer* udVertex_CreateVertexBuffer(udArrayDataFormat elements[], size_t numElements)
{
  return nullptr;
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
void udVertex_DestroyArrayBuffer(udArrayBuffer **ppVB)
{
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
void udVertex_SetArrayBufferData(udArrayBuffer *pVB, const void *pVertexData, size_t bufferLen)
{
}

#endif // UDRENDER_DRIVER == UDDRIVER_OPENGL
