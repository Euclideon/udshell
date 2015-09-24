#include "hal/driver.h"

#if EPRENDER_DRIVER == EPDRIVER_NULL

#include "hal/render.h"
#include "hal/shader.h"

// ***************************************************************************************
// Author: Manu Evans, May 2015
void epGPU_RenderVertices(epShaderProgram *pProgram, epFormatDeclaration *pVertexDecl, epArrayBuffer *pVB[], epPrimitiveType primType, size_t vertexCount, size_t firstVertex)
{
}

void epGPU_RenderIndices(epShaderProgram *pProgram, epFormatDeclaration *pVertexDecl, epArrayBuffer *pVB[], epArrayBuffer *pIB, epPrimitiveType primType, size_t indexCount,
  size_t epUnusedParam(firstIndex), size_t epUnusedParam(firstVertex))
{
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
void epGPU_RenderRanges(epShaderProgram *pProgram, epFormatDeclaration *pVertexDecl, epArrayBuffer *pVB[], epPrimitiveType primType,
  epVertexRange *pRanges, size_t rangeCount, PrimCallback *pCallback, void *pCallbackData)
{
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
void epGPU_Init()
{
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
void epGPU_Deinit()
{
}

#endif // EPRENDER_DRIVER == EPDRIVER_OPENGL
