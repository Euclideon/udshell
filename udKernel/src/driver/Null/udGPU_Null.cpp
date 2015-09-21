#include "hal/driver.h"

#if UDRENDER_DRIVER == UDDRIVER_NULL

#include "hal/render.h"
#include "hal/shader.h"

// ***************************************************************************************
// Author: Manu Evans, May 2015
void udGPU_RenderVertices(udShaderProgram *pProgram, udFormatDeclaration *pVertexDecl, udArrayBuffer *pVB[], udPrimitiveType primType, size_t vertexCount, size_t firstVertex)
{
}

void udGPU_RenderIndices(udShaderProgram *pProgram, udFormatDeclaration *pVertexDecl, udArrayBuffer *pVB[], udArrayBuffer *pIB, udPrimitiveType primType, size_t indexCount,
  size_t udUnusedParam(firstIndex), size_t udUnusedParam(firstVertex))
{
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
void udGPU_RenderRanges(udShaderProgram *pProgram, udFormatDeclaration *pVertexDecl, udArrayBuffer *pVB[], udPrimitiveType primType,
  udVertexRange *pRanges, size_t rangeCount, PrimCallback *pCallback, void *pCallbackData)
{
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
void udGPU_Init()
{
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
void udGPU_Deinit()
{
}

#endif // UDRENDER_DRIVER == UDDRIVER_OPENGL
