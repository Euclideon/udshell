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

static int s_PrimTypes[] =
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
} s_DataFormat[] =
{
  { 4, GL_FLOAT, GL_FALSE }, // udVDF_Float4,
  { 3, GL_FLOAT, GL_FALSE }, // udVDF_Float3,
  { 2, GL_FLOAT, GL_FALSE }, // udVDF_Float2,
  { 1, GL_FLOAT, GL_FALSE }, // udVDF_Float1,
  { 4, GL_UNSIGNED_BYTE, GL_TRUE },  // udVDF_UByte4N_RGBA,
  { GL_BGRA, GL_UNSIGNED_BYTE, GL_TRUE },  // udVDF_UByte4N_BGRA,
};

char s_attribNames[][16] =
{
  // MAD HAX: we reserved an extra '\0' on the end of each attrib name
  //          we'll overwrite that with the stream index temporarily...
  "a_position\0",
  "a_normal\0",
  "a_colour\0",
  "a_texcoord\0"
};

const int s_attribNameLen[] = // HAX: this length is used to overwrite the '\0' with an index at runtime
{
  10,
  8,
  8,
  10
};


// ***************************************************************************************
void udGPU_RenderVertices(udShaderProgram *pProgram, udVertexBuffer *pVB, udPrimitiveType primType, size_t vertexCount, size_t firstVertex)
{
  udVertexRange r;
  r.firstVertex = (uint32_t)firstVertex;
  r.vertexCount = (uint32_t)vertexCount;
  udGPU_RenderRanges(pProgram, pVB, primType, &r, 1);
}

// ***************************************************************************************
void udGPU_RenderRanges(struct udShaderProgram *pProgram, struct udVertexBuffer *pVB, udPrimitiveType primType, udVertexRange *pRanges, size_t rangeCount, PrimCallback *pCallback, void *pCallbackData)
{
  udVertexElement *pElements = pVB->pVertexDeclaration->pElements;
  udVertexElementData *pElementData = pVB->pVertexDeclaration->pElementData;

  // bind the buffer
  pVB->pVB->bind();

  // bind the vertex streams to the shader attributes
  int attribs[16];
  for (int a = 0; a < pVB->pVertexDeclaration->numElements; ++a)
  {
    udVertexElementType type = pElements[a].type;

    // MAD HAX: we lookup the attributes by name each render
    //          this is thoroughly lame, and should be cached on shader creation
    if (pElements[a].index == 0)
      attribs[a] = pProgram->pProgram->attributeLocation(s_attribNames[type]);
    if (pElements[a].index > 0 || attribs[a] == -1)
    {
      // MAD HAX: we reserved an extra '\0' on the end of each attrib name
      //          we'll overwrite that with the stream index temporarily...
      s_attribNames[type][s_attribNameLen[type]] = (char)('0' + pElements[a].index);
      attribs[a] = pProgram->pProgram->attributeLocation(s_attribNames[type]);
      s_attribNames[type][s_attribNameLen[type]] = 0;
    }

    if (attribs[a] == -1)
      continue;

    udVertexDataFormatGL &f = s_DataFormat[pElements[a].format];
    s_QtGLContext.pFunc->glVertexAttribPointer(attribs[a], f.components, f.type, f.normalise, pElementData[a].stride, (GLvoid*)(size_t)pElementData[a].offset);
    pProgram->pProgram->enableAttributeArray(attribs[a]);
  }

  // issue the draw call
  for (size_t i = 0; i < rangeCount; ++i)
  {
    if (pCallback)
      pCallback(i, pCallbackData);
    s_QtGLContext.pFunc->glDrawArrays(s_PrimTypes[primType], (GLint)pRanges[i].firstVertex, (GLsizei)pRanges[i].vertexCount);
  }

  // unbind the attributes  TODO: perhaps we can remove this...?
  for (int a = 0; a<pVB->pVertexDeclaration->numElements; ++a)
  {
    if (attribs[a] != -1)
      pProgram->pProgram->disableAttributeArray(attribs[a]);
  }

  pVB->pVB->release();
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
  udDebugPrintf("udGPU_INIT()\n");

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
  udDebugPrintf("udGPU_DEINIT()\n");

  // TODO: cleanup all current gl objects?

//#if UD_DEBUG
  s_QtGLContext.pDebugger->stopLogging();
  delete s_QtGLContext.pDebugger;
  s_QtGLContext.pDebugger = nullptr;
//#endif

  s_QtGLContext.pFunc = nullptr;
}

#endif // UDRENDER_DRIVER == UDDRIVER_QT
