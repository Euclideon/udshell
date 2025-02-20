#include "driver.h"

#if EPRENDER_DRIVER == EPDRIVER_NULL

#include "hal/render.h"
#include "hal/shader.h"

// ***************************************************************************************
// Author: Manu Evans, Nov 2015
void epGPU_Clear(uint32_t clearBits, float *pColor, float depth, int stencil)
{
}

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
epSyncPoint *epGPU_CreateSyncPoint()
{
  return nullptr;
}

// ***************************************************************************************
void epGPU_WaitSync(epSyncPoint **ppSync)
{
  epUnused(ppSync);
}

// ***************************************************************************************
void epGPU_DestroySyncPoint(epSyncPoint **ppSync)
{
  epUnused(ppSync);
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

// ***************************************************************************************
// Author: David Ely, August 2016
void epRenderState_SetCullMode(bool enable, epFace mode)
{
  epUnused(enable);
  epUnused(mode);
}

// ***************************************************************************************
// Author: David Ely, August 2016
void epRenderState_SetDepthCompare(bool enable, epCompareFunc func)
{
  epUnused(enable);
  epUnused(func);
}

// ***************************************************************************************
// Author: David Ely, August 2016
void epRenderState_SetDepthMask(bool flag)
{
  epUnused(flag);
}

// ***************************************************************************************
// Author: David Ely, August 2016
void epRenderState_SetBlendMode(bool enable, epBlendMode mode)
{
  epUnused(enable);
  epUnused(mode);
}

// ***************************************************************************************
// Author: David Ely, August 2016
void epRenderState_SetColorMask(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
  epUnused(r); epUnused(g); epUnused(b); epUnused(a);
}

#else
EPEMPTYFILE
#endif // EPRENDER_DRIVER == EPDRIVER_OPENGL
