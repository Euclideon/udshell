#include "hal/driver.h"

#if EPRENDER_DRIVER == EPDRIVER_NULL

#include "hal/vertex.h"

// ***************************************************************************************
// Author: Manu Evans, May 2015
epFormatDeclaration *epVertex_CreateFormatDeclaration(const epArrayElement *pElementArray, int elementCount)
{
  return nullptr;
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
void epVertex_DestroyFormatDeclaration(epFormatDeclaration **ppDeclaration)
{
}

// ***************************************************************************************
// Author: Manu Evans, Aug 2015
epArrayBuffer* epVertex_CreateIndexBuffer(epArrayDataFormat format)
{
  return nullptr;
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
epArrayBuffer* epVertex_CreateVertexBuffer(epArrayDataFormat elements[], size_t numElements)
{
  return nullptr;
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
void epVertex_DestroyArrayBuffer(epArrayBuffer **ppVB)
{
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
void epVertex_SetArrayBufferData(epArrayBuffer *pVB, const void *pVertexData, size_t bufferLen)
{
}

#else
EPEMPTYFILE
#endif // EPRENDER_DRIVER == EPDRIVER_OPENGL
