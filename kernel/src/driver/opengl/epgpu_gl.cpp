#include "hal/driver.h"

#if EPRENDER_DRIVER == EPDRIVER_OPENGL

#include "hal/render.h"
#include "hal/shader.h"
#include "hal/vertex.h"

#include "ep_opengl.h"


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
  { 4,        GL_FLOAT,         GL_FALSE }, // epVDF_Float4
  { 3,        GL_FLOAT,         GL_FALSE }, // epVDF_Float3
  { 2,        GL_FLOAT,         GL_FALSE }, // epVDF_Float2
  { 1,        GL_FLOAT,         GL_FALSE }, // epVDF_Float1
  { 4,        GL_UNSIGNED_BYTE, GL_TRUE },  // epVDF_UByte4N_RGBA
  { GL_BGRA,  GL_UNSIGNED_BYTE, GL_TRUE },  // epVDF_UByte4N_BGRA
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


// ***************************************************************************************
// Author: Manu Evans, May 2015
void epGPU_RenderVertices(epShaderProgram *pProgram, epFormatDeclaration *pVertexDecl, epArrayBuffer *pVB[], epPrimitiveType primType, size_t vertexCount, size_t firstVertex)
{
  epVertexRange r;
  r.firstVertex = (uint32_t)firstVertex;
  r.vertexCount = (uint32_t)vertexCount;
  epGPU_RenderRanges(pProgram, pVertexDecl, pVB, primType, &r, 1);
}

// ***************************************************************************************
// Author: Manu Evans, Aug 2015
void epGPU_RenderIndices(epShaderProgram *pProgram, epFormatDeclaration *pVertexDecl, epArrayBuffer *pVB[], epArrayBuffer *pIB, epPrimitiveType primType, size_t indexCount, size_t epUnusedParam(firstIndex), size_t epUnusedParam(firstVertex))
{
  udArrayElement *pElements = pVertexDecl->pElements;
  epArrayElementData *pElementData = pVertexDecl->pElementData;

  // bind the vertex streams to the shader attributes
  GLint attribs[16];
  bool boundVB[16] = { false };
  for (int a = 0; a<pVertexDecl->numElements; ++a)
  {
    attribs[a] = glGetAttribLocation(pProgram->program, pElements[a].attributeName);
    if (attribs[a] == -1)
      continue;

    if (!boundVB[pElements[a].stream])
    {
      // bind the buffer
      glBindBuffer(GL_ARRAY_BUFFER, pVB[pElements[a].stream]->buffer);
      boundVB[pElements[a].stream] = true;
    }

    epVertexDataFormatGL &f = s_dataFormat[pElements[a].format];
    glVertexAttribPointer(attribs[a], f.components, f.type, f.normalise, pElementData[a].stride, (GLvoid*)(size_t)pElementData[a].offset);
    glEnableVertexAttribArray(attribs[a]);
  }

  // issue the draw call
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pIB->buffer);
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
      type = GL_UNSIGNED_SHORT;
      EPASSERT(false, "Invalid index buffer type!");
      break;
  }
  glDrawElements(s_primTypes[primType], (GLsizei)indexCount, type, nullptr);

  // unbind the attributes  TODO: perhaps we can remove this...?
  for (int a = 0; a<pVertexDecl->numElements; ++a)
  {
    if (attribs[a] != -1)
      glDisableVertexAttribArray(attribs[a]);
//    if (boundVB[pElements[a].stream])
//    {
//      pVB[pElements[a].stream]->pVB->release();
//      boundVB[pElements[a].stream] = false;
//    }
  }
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
void epGPU_RenderRanges(epShaderProgram *pProgram, epFormatDeclaration *pVertexDecl, epArrayBuffer *pVB[], epPrimitiveType primType, epVertexRange *pRanges, size_t rangeCount, PrimCallback *pCallback, void *pCallbackData)
{
  udArrayElement *pElements = pVertexDecl->pElements;
  epArrayElementData *pElementData = pVertexDecl->pElementData;

  // bind the vertex streams to the shader attributes
  GLint attribs[16];
  bool boundVB[16] = { false };
  for (int a = 0; a<pVertexDecl->numElements; ++a)
  {
    attribs[a] = glGetAttribLocation(pProgram->program, pElements[a].attributeName);
    if (attribs[a] == -1)
      continue;

    if (!boundVB[pElements[a].stream])
    {
      // bind the buffer
      glBindBuffer(GL_ARRAY_BUFFER, pVB[pElements[a].stream]->buffer);
      boundVB[pElements[a].stream] = true;
    }

    epVertexDataFormatGL &f = s_dataFormat[pElements[a].format];
    glVertexAttribPointer(attribs[a], f.components, f.type, f.normalise, pElementData[a].stride, (GLvoid*)(size_t)pElementData[a].offset);
    glEnableVertexAttribArray(attribs[a]);
  }

  // issue the draw call
  for (size_t i=0; i<rangeCount; ++i)
  {
    if (pCallback)
      pCallback(i, pCallbackData);
    glDrawArrays(s_primTypes[primType], (GLint)pRanges[i].firstVertex, (GLsizei)pRanges[i].vertexCount);
  }

  // unbind the attributes  TODO: perhaps we can remove this...?
  for (int a = 0; a<pVertexDecl->numElements; ++a)
  {
    if (attribs[a] != -1)
      glDisableVertexAttribArray(attribs[a]);
//    if (boundVB[pElements[a].stream])
//    {
//      pVB[pElements[a].stream]->pVB->release();
//      boundVB[pElements[a].stream] = false;
//    }
  }
}


// ***************************************************************************************
// Author: Manu Evans, May 2015
void epGPU_Init()
{
#if defined(USE_GLEW)
  glewInit();
#endif
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
void epGPU_Deinit()
{
}

#endif // EPRENDER_DRIVER == EPDRIVER_OPENGL
