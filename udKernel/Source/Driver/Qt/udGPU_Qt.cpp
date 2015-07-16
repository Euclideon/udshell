#include "udDriver.h"

#if UDRENDER_DRIVER == UDDRIVER_QT

#include "udGPU.h"
#include "udShader.h"
#include "udVertex.h"

#include "udQt_Internal.h"

#include <QOpenGLContext>

// for convenience we keep track of Qt GL context info in this struct
udQtGLContext s_QtGLContext =
{
  nullptr,  // pFunc
  nullptr,  // pDebugger
};


// ***************************************************************************************
void udGPU_RenderVertices(udShaderProgram *pProgram, udVertexBuffer *pVB, udPrimitiveType primType, size_t vertexCount, size_t firstVertex)
{
}

// ***************************************************************************************
void udGPU_RenderRanges(struct udShaderProgram *pProgram, struct udVertexBuffer *pVB, udPrimitiveType primType, udVertexRange *pRanges, size_t rangeCount, PrimCallback *pCallback, void *pCallbackData)
{
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
  udDebugPrintf("\nudGPU_INIT()\n");

  // TODO: gracefully handle this case, maybe try to create a context or postpone init?
  UDASSERT(QOpenGLContext::currentContext(), "QOpenGLContext::currentContext() should not be null when we call udGPU_Init");

  s_QtGLContext.pFunc = QOpenGLContext::currentContext()->versionFunctions<QOpenGLFunctions_2_0>();
  // TODO: we depend on OpenGL 2.0, should we fallback? or gracefully handle
  UDASSERT(s_QtGLContext.pFunc, "we expect QOpenGLFunctions_2_0 to be available");

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
  udDebugPrintf("\nudGPU_DEINIT()\n");

//#if UD_DEBUG
  s_QtGLContext.pDebugger->stopLogging();
  delete s_QtGLContext.pDebugger;
  s_QtGLContext.pDebugger = nullptr;
//#endif

  s_QtGLContext.pFunc = nullptr;
}

#endif // UDRENDER_DRIVER == UDDRIVER_QT
