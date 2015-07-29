#include "udDriver.h"

#if UDRENDER_DRIVER == UDDRIVER_NULL

#include "udVertex.h"

// ***************************************************************************************
// Author: Manu Evans, May 2015
udVertexDeclaration *udVertex_CreateVertexDeclaration(const udVertexElement *pElementArray, int elementCount)
{
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
void udVertex_DestroyVertexDeclaration(udVertexDeclaration **ppDeclaration)
{
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
udVertexBuffer* udVertex_CreateVertexBuffer(udVertexDeclaration *pFormat)
{
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
void udVertex_DestroyVertexBuffer(udVertexBuffer **ppVB)
{
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
void udVertex_SetVertexBufferData(udVertexBuffer *pVB, void *pVertexData, size_t bufferLen)
{
}

#endif // UDRENDER_DRIVER == UDDRIVER_OPENGL
