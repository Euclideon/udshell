#include "hal/driver.h"

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
  nullptr,  // pFunc
  nullptr   // pFunc3_2_Core
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
void epGPU_Clear(double color[4], double depth, int stencil)
{
  s_QtGLContext.pFunc->glClearColor(color[0], color[1], color[2], color[3]);
  s_QtGLContext.pFunc->glClearDepthf(depth);
  s_QtGLContext.pFunc->glClearStencil(stencil);
  s_QtGLContext.pFunc->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

// ***************************************************************************************
void epGPU_RenderVertices(epShaderProgram *pProgram, epFormatDeclaration *pVertexDecl, epArrayBuffer *pVB[], epPrimitiveType primType, size_t vertexCount, size_t firstVertex)
{
  epVertexRange r;
  r.firstVertex = (uint32_t)firstVertex;
  r.vertexCount = (uint32_t)vertexCount;
  epGPU_RenderRanges(pProgram, pVertexDecl, pVB, primType, &r, 1);
}

// ***************************************************************************************
void epGPU_RenderIndices(epShaderProgram *pProgram, epFormatDeclaration *pVertexDecl, epArrayBuffer *pVB[], epArrayBuffer *pIB, epPrimitiveType primType, size_t indexCount, size_t epUnusedParam(firstIndex), size_t epUnusedParam(firstVertex))
{
  epArrayElement *pElements = pVertexDecl->pElements;
  epArrayElementData *pElementData = pVertexDecl->pElementData;

  // bind the vertex streams to the shader attributes
  int attribs[16];
  bool boundVB[16] = { false };
  for (int a = 0; a < pVertexDecl->numElements; ++a)
  {
    attribs[a] = pProgram->pProgram->attributeLocation(pElements[a].attributeName);
    if (attribs[a] == -1)
      continue;

    if (!boundVB[pElements[a].stream])
    {
      // bind the buffer
      pVB[pElements[a].stream]->pBuffer->bind();
      boundVB[pElements[a].stream] = true;
    }

    epVertexDataFormatGL &f = s_dataFormat[pElements[a].format];
    s_QtGLContext.pFunc->glVertexAttribPointer(attribs[a], f.components, f.type, f.normalise, pElementData[a].stride, (GLvoid*)(size_t)pElementData[a].offset);
    pProgram->pProgram->enableAttributeArray(attribs[a]);
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

  // unbind the attributes  TODO: perhaps we can remove this...?
  for (int a = 0; a<pVertexDecl->numElements; ++a)
  {
    if (attribs[a] != -1)
      pProgram->pProgram->disableAttributeArray(attribs[a]);
    if (boundVB[pElements[a].stream])
    {
      pVB[pElements[a].stream]->pBuffer->release();
      boundVB[pElements[a].stream] = false;
    }
  }
}

// ***************************************************************************************
void epGPU_RenderRanges(epShaderProgram *pProgram, epFormatDeclaration *pVertexDecl, epArrayBuffer *pVB[], epPrimitiveType primType, epVertexRange *pRanges, size_t rangeCount, PrimCallback *pCallback, void *pCallbackData)
{
  epArrayElement *pElements = pVertexDecl->pElements;
  epArrayElementData *pElementData = pVertexDecl->pElementData;

  // bind the vertex streams to the shader attributes
  int attribs[16];
  bool boundVB[16] = { false };
  for (int a = 0; a < pVertexDecl->numElements; ++a)
  {
    attribs[a] = pProgram->pProgram->attributeLocation(pElements[a].attributeName);
    if (attribs[a] == -1)
      continue;

    if (!boundVB[pElements[a].stream])
    {
      // bind the buffer
      pVB[pElements[a].stream]->pBuffer->bind();
      boundVB[pElements[a].stream] = true;
    }

    epVertexDataFormatGL &f = s_dataFormat[pElements[a].format];
    s_QtGLContext.pFunc->glVertexAttribPointer(attribs[a], f.components, f.type, f.normalise, pElementData[a].stride, (GLvoid*)(size_t)pElementData[a].offset);
    pProgram->pProgram->enableAttributeArray(attribs[a]);
  }

  // issue the draw call
  for (size_t i = 0; i < rangeCount; ++i)
  {
    if (pCallback)
      pCallback(i, pCallbackData);
    s_QtGLContext.pFunc->glDrawArrays(s_primTypes[primType], (GLint)pRanges[i].firstVertex, (GLsizei)pRanges[i].vertexCount);
  }

  // unbind the attributes  TODO: perhaps we can remove this...?
  for (int a = 0; a<pVertexDecl->numElements; ++a)
  {
    if (attribs[a] != -1)
      pProgram->pProgram->disableAttributeArray(attribs[a]);
    if (boundVB[pElements[a].stream])
    {
      pVB[pElements[a].stream]->pBuffer->release();
      boundVB[pElements[a].stream] = false;
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
  EPASSERT(QOpenGLContext::currentContext(), "QOpenGLContext::currentContext() should not be null when we call epGPU_Init");

  s_QtGLContext.pFunc = QOpenGLContext::currentContext()->functions();

  // NOTE: if we're using an old or incompatible version of GL, this will set our pointer to null.
  // This should only be used for 3.2+ functionality
  s_QtGLContext.pFunc3_2_Core = QOpenGLContext::currentContext()->versionFunctions<QOpenGLFunctions_3_2_Core>();

  if (s_QtGLContext.pFunc3_2_Core)
    s_QtGLContext.pFunc3_2_Core->initializeOpenGLFunctions();
}

// ***************************************************************************************
void epGPU_Deinit()
{
  // TODO: cleanup all current gl objects?

  s_QtGLContext.pFunc = nullptr;
}

#else
EPEMPTYFILE
#endif // EPRENDER_DRIVER == EPDRIVER_QT
