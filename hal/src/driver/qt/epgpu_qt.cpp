#include "driver.h"

#if EPRENDER_DRIVER == EPDRIVER_QT

#include "hal/render.h"
#include "hal/shader.h"
#include "hal/vertex.h"

#include "eprender_qt.h"

#include <QOpenGLContext>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>

// for convenience we keep track of Qt GL context info in this struct
epQtGLContext s_QtGLContext =
{
  nullptr, // pFunc
  nullptr, // pFunc3_2_Core
  nullptr  // pFunc4_0_Core
};

static int s_primTypes[] =
{
  GL_POINTS,
  GL_LINES,
  GL_LINE_STRIP,
  GL_TRIANGLES,
  GL_TRIANGLE_STRIP,
  GL_TRIANGLE_FAN
};

struct epVertexDataFormatGL
{
  GLint components;
  GLenum type;
  GLboolean normalise;
} s_dataFormat[] =
{
  { 4, GL_FLOAT, GL_FALSE }, // epVDF_Float4
  { 3, GL_FLOAT, GL_FALSE }, // epVDF_Float3
  { 2, GL_FLOAT, GL_FALSE }, // epVDF_Float2
  { 1, GL_FLOAT, GL_FALSE }, // epVDF_Float
  { 4, GL_UNSIGNED_BYTE, GL_TRUE },       // epVDF_UByte4N_RGBA
  { GL_BGRA, GL_UNSIGNED_BYTE, GL_TRUE }, // epVDF_UByte4N_BGRA
  { 4, GL_INT, GL_FALSE }, // epVDF_Int4
  { 3, GL_INT, GL_FALSE }, // epVDF_Int3
  { 2, GL_INT, GL_FALSE }, // epVDF_Int2
  { 1, GL_INT, GL_FALSE }, // epVDF_Int
  { 4, GL_UNSIGNED_INT, GL_FALSE }, // epVDF_UInt4
  { 3, GL_UNSIGNED_INT, GL_FALSE }, // epVDF_UInt3
  { 2, GL_UNSIGNED_INT, GL_FALSE }, // epVDF_UInt2
  { 1, GL_UNSIGNED_INT, GL_FALSE }, // epVDF_UInt
  { 4, GL_SHORT, GL_FALSE }, // epVDF_Short4
  { 2, GL_SHORT, GL_FALSE }, // epVDF_Short2
  { 4, GL_SHORT, GL_TRUE },  // epVDF_Short4N
  { 2, GL_SHORT, GL_TRUE },  // epVDF_Short2N
  { 1, GL_SHORT, GL_FALSE }, // epVDF_Short
  { 4, GL_UNSIGNED_SHORT, GL_FALSE }, // epVDF_UShort4
  { 2, GL_UNSIGNED_SHORT, GL_FALSE }, // epVDF_UShort2
  { 4, GL_UNSIGNED_SHORT, GL_TRUE },  // epVDF_UShort4N
  { 2, GL_UNSIGNED_SHORT, GL_TRUE },  // epVDF_UShort2N
  { 1, GL_UNSIGNED_SHORT, GL_FALSE }, // epVDF_UShort
  { 4, GL_BYTE, GL_FALSE },           // epVDF_Byte4
  { 4, GL_UNSIGNED_BYTE, GL_FALSE },  // epVDF_UByte4
  { 4, GL_BYTE, GL_TRUE },            // epVDF_Byte4N
  { 1, GL_BYTE, GL_FALSE },           // epVDF_Byte
  { 1, GL_UNSIGNED_BYTE, GL_FALSE },  // epVDF_UByte
};

struct epSyncPoint
{
  GLsync syncId;
};

// ***************************************************************************************
// Author: Manu Evans, Nov 2015
void epGPU_Clear(uint32_t clearBits, float *pColor, float depth, int stencil)
{
  GLbitfield mask = 0;
  if (clearBits & epC_Color)
  {
    mask |= GL_COLOR_BUFFER_BIT;
    s_QtGLContext.pFunc->glClearColor(pColor[0], pColor[1], pColor[2], pColor[3]);
  }
  if (clearBits & epC_Depth)
  {
    mask |= GL_DEPTH_BUFFER_BIT;
    s_QtGLContext.pFunc->glClearDepthf(depth);
  }
  if (clearBits & epC_Stencil)
  {
    mask |= GL_STENCIL_BUFFER_BIT;
    s_QtGLContext.pFunc->glClearStencil(stencil);
  }
  s_QtGLContext.pFunc->glClear(mask);
}

// ***************************************************************************************
void epGPU_RenderVertices(epShaderProgram *pProgram, epShaderInputConfig *pConfig, epArrayBuffer *pVB[], epPrimitiveType primType, size_t vertexCount, size_t firstVertex)
{
  epVertexRange r;
  r.firstVertex = (uint32_t)firstVertex;
  r.vertexCount = (uint32_t)vertexCount;
  epGPU_RenderRanges(pProgram, pConfig, pVB, primType, &r, 1);
}

// ***************************************************************************************
void epGPU_RenderIndices(epShaderProgram *pProgram, epShaderInputConfig *pConfig, epArrayBuffer *pVB[], epArrayBuffer *pIB, epPrimitiveType primType, size_t indexCount, size_t epUnusedParam(firstIndex), size_t epUnusedParam(firstVertex))
{
  epArrayElement *pElements = pConfig->pElements;
  epArrayElementData *pElementData = pConfig->pElementData;

  bool boundVB[16] = { false };
  for (int i = 0; i < pConfig->numElements; ++i)
  {
    if (!boundVB[pElements[i].stream])
    {
      pVB[pElements[i].stream]->pBuffer->bind();
      boundVB[pElements[i].stream] = true;
    }

    epVertexDataFormatGL &f = s_dataFormat[pElements[i].format];
    s_QtGLContext.pFunc->glVertexAttribPointer(pElementData[i].attribLocation, f.components, f.type, f.normalise, pElements[i].stride, (GLvoid*)(size_t)pElements[i].offset);
    pProgram->pProgram->enableAttributeArray(pElementData[i].attribLocation);
  }

  // issue the draw call
  pIB->pBuffer->bind();
  GLenum type;
  switch (pIB->pFormat[0])
  {
  case epVDF_UInt:
    type = GL_UNSIGNED_INT; break;
  case epVDF_UShort:
    type = GL_UNSIGNED_SHORT; break;
  case epVDF_UByte:
    type = GL_UNSIGNED_BYTE; break;
  default:
    type = GL_UNSIGNED_INT;
    EPASSERT(false, "Invalid index buffer type!");
  }

  s_QtGLContext.pFunc->glDrawElements(s_primTypes[primType], (GLsizei)indexCount, type, nullptr);
  pIB->pBuffer->release();

  for (int i = 0; i < pConfig->numElements; ++i)
  {
    pProgram->pProgram->disableAttributeArray(pElementData[i].attribLocation);
    if (boundVB[pElements[i].stream])
    {
      pVB[pElements[i].stream]->pBuffer->release();
      boundVB[pElements[i].stream] = false;
    }
  }
}

// ***************************************************************************************
void epGPU_RenderRanges(epShaderProgram *pProgram, epShaderInputConfig *pConfig, epArrayBuffer *pVB[], epPrimitiveType primType, epVertexRange *pRanges, size_t rangeCount, PrimCallback *pCallback, void *pCallbackData)
{
  epArrayElement *pElements = pConfig->pElements;
  epArrayElementData *pElementData = pConfig->pElementData;

  bool boundVB[16] = { false };
  for (int i = 0; i < pConfig->numElements; ++i)
  {
    if (!boundVB[pElements[i].stream])
    {
      pVB[pElements[i].stream]->pBuffer->bind();
      boundVB[pElements[i].stream] = true;
    }

    epVertexDataFormatGL &f = s_dataFormat[pElements[i].format];
    s_QtGLContext.pFunc->glVertexAttribPointer(pElementData[i].attribLocation, f.components, f.type, f.normalise, pElements[i].stride, (GLvoid*)(size_t)pElements[i].offset);
    pProgram->pProgram->enableAttributeArray(pElementData[i].attribLocation);
  }

  // issue the draw call
  for (size_t i = 0; i < rangeCount; ++i)
  {
    if (pCallback)
      pCallback(i, pCallbackData);
    s_QtGLContext.pFunc->glDrawArrays(s_primTypes[primType], (GLint)pRanges[i].firstVertex, (GLsizei)pRanges[i].vertexCount);
  }

  for (int i = 0; i < pConfig->numElements; ++i)
  {
    pProgram->pProgram->disableAttributeArray(pElementData[i].attribLocation);
    if (boundVB[pElements[i].stream])
    {
      pVB[pElements[i].stream]->pBuffer->release();
      boundVB[pElements[i].stream] = false;
    }
  }
}

// ***************************************************************************************
epSyncPoint *epGPU_CreateSyncPoint()
{
  if (!s_QtGLContext.pFunc3_2_Core)
  {
    // if we don't have at least 3.2, then just call glFinish(); this will block
    s_QtGLContext.pFunc->glFinish();
    return nullptr;
  }

  epSyncPoint *pSync = epAllocType(epSyncPoint, 1, epAF_None);
  pSync->syncId = s_QtGLContext.pFunc3_2_Core->glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
  return pSync;
}

// ***************************************************************************************
void epGPU_WaitSync(epSyncPoint **ppSync)
{
  if (!s_QtGLContext.pFunc3_2_Core || !(*ppSync))
    return;

  s_QtGLContext.pFunc->glFlush();
  s_QtGLContext.pFunc3_2_Core->glWaitSync((*ppSync)->syncId, 0, GL_TIMEOUT_IGNORED);
  epGPU_DestroySyncPoint(ppSync);
}

void epGPU_DestroySyncPoint(epSyncPoint **ppSync)
{
  s_QtGLContext.pFunc3_2_Core->glDeleteSync((*ppSync)->syncId);
  epFree(*ppSync);
  *ppSync = nullptr;
}

// ***************************************************************************************
void epGPU_Init()
{
  // TODO: gracefully handle this case, maybe try to create a context or postpone init?
  //EPASSERT(QOpenGLContext::currentContext(), "QOpenGLContext::currentContext() should not be null when we call epGPU_Init");
  EPASSERT(s_QtGLContext.pContext, "s_QtGLContext.pResourceContext should not be null when we call epGPU_Init");

  s_QtGLContext.pFunc = s_QtGLContext.pContext->functions();

  // NOTE: if we're using an old or incompatible version of GL, this will set our pointer to null.
  // This should only be used for 4.0+ functionality
  s_QtGLContext.pFunc4_0_Core = s_QtGLContext.pContext->versionFunctions<QOpenGLFunctions_4_0_Core>();

  if (s_QtGLContext.pFunc4_0_Core)
    s_QtGLContext.pFunc4_0_Core->initializeOpenGLFunctions();

  // NOTE: if we're using an old or incompatible version of GL, this will set our pointer to null.
  // This should only be used for 3.2+ functionality
  s_QtGLContext.pFunc3_2_Core = s_QtGLContext.pContext->versionFunctions<QOpenGLFunctions_3_2_Core>();

  if (s_QtGLContext.pFunc3_2_Core)
    s_QtGLContext.pFunc3_2_Core->initializeOpenGLFunctions();
}

// ***************************************************************************************
void epGPU_Deinit()
{
  // TODO: cleanup all current gl objects?

  s_QtGLContext.pFunc = nullptr;
  s_QtGLContext.pFunc3_2_Core = nullptr;
  s_QtGLContext.pFunc4_0_Core = nullptr;

  s_QtGLContext.pContext = nullptr;
  s_QtGLContext.pSurface = nullptr;
}

// ***************************************************************************************
// Author: David Ely, August 2016
void epRenderState_SetDepthCompare(bool enable, epCompareFunc func)
{
  if (enable)
  {
    s_QtGLContext.pFunc->glEnable(GL_DEPTH_TEST);
    s_QtGLContext.pFunc->glDepthFunc(GL_NEVER + (GLenum)func);
  }
  else
  {
    s_QtGLContext.pFunc->glDisable(GL_DEPTH_TEST);
  }
}

// ***************************************************************************************
// Author: David Ely, August 2016
void epRenderState_SetDepthMask(bool flag)
{
  s_QtGLContext.pFunc->glDepthMask(flag ? GL_TRUE : GL_FALSE);
}

// ***************************************************************************************
// Author: David Ely, August 2016
void epRenderState_SetCullMode(bool enable, epFace mode)
{
  if (enable)
  {
    static int s_lookup[] =
    {
      GL_FRONT,
      GL_BACK,
      GL_FRONT_AND_BACK
    };
    s_QtGLContext.pFunc->glEnable(GL_CULL_FACE);
    s_QtGLContext.pFunc->glFrontFace(GL_CW);
    s_QtGLContext.pFunc->glCullFace(s_lookup[mode]);
  }
  else
  {
    s_QtGLContext.pFunc->glDisable(GL_CULL_FACE);
  }
}

// ***************************************************************************************
// Author: David Ely, August 2016
void epRenderState_SetBlendMode(bool enable, epBlendMode mode)
{
  if (enable)
  {
    s_QtGLContext.pFunc->glEnable(GL_BLEND);
    if (mode == epBM_Alpha)
      s_QtGLContext.pFunc->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    else
      s_QtGLContext.pFunc->glBlendFunc(GL_ONE, GL_ONE);
  }
  else
  {
    s_QtGLContext.pFunc->glDisable(GL_BLEND);
  }
}

// ***************************************************************************************
// Author: David Ely, August 2016
void epRenderState_SetColorMask(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
  s_QtGLContext.pFunc->glColorMask(r, g, b, a);
}

#else
EPEMPTYFILE
#endif // EPRENDER_DRIVER == EPDRIVER_QT
