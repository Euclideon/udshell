#include "udDriver.h"

#if UDRENDER_DRIVER == UDDRIVER_NULL

#include "udGPU.h"
#include "udShader.h"

// ***************************************************************************************
// Author: Manu Evans, May 2015
void udGPU_RenderVertices(udShaderProgram *pProgram, udVertexBuffer *pVB, udPrimitiveType primType, size_t vertexCount, size_t firstVertex)
{
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
void udGPU_RenderRanges(struct udShaderProgram *pProgram, struct udVertexBuffer *pVB, udPrimitiveType primType, udVertexRange *pRanges, size_t rangeCount, PrimCallback *pCallback, void *pCallbackData)
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
