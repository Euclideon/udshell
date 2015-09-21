#include "hal/driver.h"

#if UDRENDER_DRIVER == UDDRIVER_QT

#include "hal/render.h"
#include "hal/shader.h"
#include "hal/vertex.h"

#include "udQtRender_Internal.h"

#include <QOpenGLContext>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>

// for convenience we keep track of Qt GL context info in this struct
udQtGLContext s_QtGLContext =
{
  nullptr,  // pFunc
  nullptr,  // pDebugger
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

struct udVertexDataFormatGL
{
  GLint components;
  GLenum type;
  GLboolean normalise;
} s_dataFormat[] =
{
  { 4, GL_FLOAT, GL_FALSE }, // udVDF_Float4
  { 3, GL_FLOAT, GL_FALSE }, // udVDF_Float3
  { 2, GL_FLOAT, GL_FALSE }, // udVDF_Float2
  { 1, GL_FLOAT, GL_FALSE }, // udVDF_Float
  { 4, GL_UNSIGNED_BYTE, GL_TRUE },       // udVDF_UByte4N_RGBA
  { GL_BGRA, GL_UNSIGNED_BYTE, GL_TRUE }, // udVDF_UByte4N_BGRA
  { 4, GL_INT, GL_FALSE }, // udVDF_Int4
  { 3, GL_INT, GL_FALSE }, // udVDF_Int3
  { 2, GL_INT, GL_FALSE }, // udVDF_Int2
  { 1, GL_INT, GL_FALSE }, // udVDF_Int
  { 4, GL_UNSIGNED_INT, GL_FALSE }, // udVDF_UInt4
  { 3, GL_UNSIGNED_INT, GL_FALSE }, // udVDF_UInt3
  { 2, GL_UNSIGNED_INT, GL_FALSE }, // udVDF_UInt2
  { 1, GL_UNSIGNED_INT, GL_FALSE }, // udVDF_UInt
  { 4, GL_SHORT, GL_FALSE }, // udVDF_Short4
  { 2, GL_SHORT, GL_FALSE }, // udVDF_Short2
  { 4, GL_SHORT, GL_TRUE },  // udVDF_Short4N
  { 2, GL_SHORT, GL_TRUE },  // udVDF_Short2N
  { 1, GL_SHORT, GL_FALSE }, // udVDF_Short
  { 4, GL_UNSIGNED_SHORT, GL_FALSE }, // udVDF_UShort4
  { 2, GL_UNSIGNED_SHORT, GL_FALSE }, // udVDF_UShort2
  { 4, GL_UNSIGNED_SHORT, GL_TRUE },  // udVDF_UShort4N
  { 2, GL_UNSIGNED_SHORT, GL_TRUE },  // udVDF_UShort2N
  { 1, GL_UNSIGNED_SHORT, GL_FALSE }, // udVDF_UShort
  { 4, GL_BYTE, GL_FALSE },           // udVDF_Byte4
  { 4, GL_UNSIGNED_BYTE, GL_FALSE },  // udVDF_UByte4
  { 4, GL_BYTE, GL_TRUE },            // udVDF_Byte4N
  { 1, GL_BYTE, GL_FALSE },           // udVDF_Byte
  { 1, GL_UNSIGNED_BYTE, GL_FALSE },  // udVDF_UByte
};


// ***************************************************************************************
void udGPU_RenderVertices(udShaderProgram *pProgram, udFormatDeclaration *pVertexDecl, udArrayBuffer *pVB[], udPrimitiveType primType, size_t vertexCount, size_t firstVertex)
{
  udVertexRange r;
  r.firstVertex = (uint32_t)firstVertex;
  r.vertexCount = (uint32_t)vertexCount;
  udGPU_RenderRanges(pProgram, pVertexDecl, pVB, primType, &r, 1);
}

// ***************************************************************************************
void udGPU_RenderIndices(udShaderProgram *pProgram, udFormatDeclaration *pVertexDecl, udArrayBuffer *pVB[], udArrayBuffer *pIB, udPrimitiveType primType, size_t indexCount, size_t udUnusedParam(firstIndex), size_t udUnusedParam(firstVertex))
{
  udArrayElement *pElements = pVertexDecl->pElements;
  udArrayElementData *pElementData = pVertexDecl->pElementData;

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

    udVertexDataFormatGL &f = s_dataFormat[pElements[a].format];
    s_QtGLContext.pFunc->glVertexAttribPointer(attribs[a], f.components, f.type, f.normalise, pElementData[a].stride, (GLvoid*)(size_t)pElementData[a].offset);
    pProgram->pProgram->enableAttributeArray(attribs[a]);
  }

  // issue the draw call
  pIB->pBuffer->bind();
  GLenum type;
  switch (pIB->pFormat[0])
  {
    case udVDF_UInt:
      type = GL_UNSIGNED_INT; break;
    case udVDF_UShort:
      type = GL_UNSIGNED_SHORT; break;
    case udVDF_UByte:
      type = GL_UNSIGNED_BYTE; break;
    default:
      type = GL_UNSIGNED_INT;
      UDASSERT(false, "Invalid index buffer type!");
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
void udGPU_RenderRanges(udShaderProgram *pProgram, udFormatDeclaration *pVertexDecl, udArrayBuffer *pVB[], udPrimitiveType primType, udVertexRange *pRanges, size_t rangeCount, PrimCallback *pCallback, void *pCallbackData)
{
  udArrayElement *pElements = pVertexDecl->pElements;
  udArrayElementData *pElementData = pVertexDecl->pElementData;

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

    udVertexDataFormatGL &f = s_dataFormat[pElements[a].format];
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

// ---------------------------------------------------------------------------------------
void onGLMessageLogged(QOpenGLDebugMessage message)
{
  // TODO: replace this with something better
  qDebug() << message;
}

// ***************************************************************************************
void udGPU_Init()
{
  // TODO: gracefully handle this case, maybe try to create a context or postpone init?
  UDASSERT(QOpenGLContext::currentContext(), "QOpenGLContext::currentContext() should not be null when we call udGPU_Init");

  s_QtGLContext.pFunc = QOpenGLContext::currentContext()->versionFunctions<QOpenGLFunctions_2_0>();
  // TODO: we depend on OpenGL 2.0, should we fallback? or gracefully handle
  UDASSERT(s_QtGLContext.pFunc, "we expect QOpenGLFunctions_2_0 to be available");

  // TODO: Override the QML GL version??
  // TODO: Create a share GL Context that we have control over?
  udDebugPrintf("GL VERSION: %d.%d\n",
    QOpenGLContext::currentContext()->format().majorVersion(), QOpenGLContext::currentContext()->format().minorVersion());

  // TODO: this might be redundant
  s_QtGLContext.pFunc->initializeOpenGLFunctions();

//#if UD_DEBUG
  // TODO: kill this in release builds?
  // TODO: make better usage of this debugger - hook into our logging system?
  s_QtGLContext.pDebugger = new QOpenGLDebugLogger();
  QObject::connect(s_QtGLContext.pDebugger, &QOpenGLDebugLogger::messageLogged, &onGLMessageLogged);

  if (s_QtGLContext.pDebugger->initialize())
  {
    // TODO: Synchronous Logging has a high overhead but ensures messages are received in order
    s_QtGLContext.pDebugger->startLogging(QOpenGLDebugLogger::SynchronousLogging);
    s_QtGLContext.pDebugger->enableMessages();
  }
//#endif
}

// ***************************************************************************************
void udGPU_Deinit()
{
  // TODO: cleanup all current gl objects?

//#if UD_DEBUG
  s_QtGLContext.pDebugger->stopLogging();
  delete s_QtGLContext.pDebugger;
  s_QtGLContext.pDebugger = nullptr;
//#endif

  s_QtGLContext.pFunc = nullptr;
}

#endif // UDRENDER_DRIVER == UDDRIVER_QT
