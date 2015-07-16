#include "udDriver.h"

#if UDRENDER_DRIVER == UDDRIVER_QT

#include "udVertex.h"


// ***************************************************************************************
udVertexDeclaration *udVertex_CreateVertexDeclaration(const udVertexElement *pElementArray, int elementCount)
{
  return 0;
}

// ***************************************************************************************
void udVertex_DestroyVertexDeclaration(udVertexDeclaration **ppDeclaration)
{
}

// ***************************************************************************************
udVertexBuffer* udVertex_CreateVertexBuffer(udVertexDeclaration *pFormat)
{
  return 0;
}

// ***************************************************************************************
void udVertex_DestroyVertexBuffer(udVertexBuffer **ppVB)
{
}

// ***************************************************************************************
void udVertex_SetVertexBufferData(udVertexBuffer *pVB, void *pVertexData, size_t bufferLen)
{
}


#endif // UDRENDER_DRIVER == UDDRIVER_QT
