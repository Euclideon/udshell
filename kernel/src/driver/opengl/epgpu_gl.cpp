#include "hal/driver.h"

#if UDRENDER_DRIVER == UDDRIVER_OPENGL

#include "hal/render.h"
#include "hal/shader.h"
#include "hal/vertex.h"

#include "udOpenGL_Internal.h"


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
  { 4,        GL_FLOAT,         GL_FALSE }, // udVDF_Float4
  { 3,        GL_FLOAT,         GL_FALSE }, // udVDF_Float3
  { 2,        GL_FLOAT,         GL_FALSE }, // udVDF_Float2
  { 1,        GL_FLOAT,         GL_FALSE }, // udVDF_Float1
  { 4,        GL_UNSIGNED_BYTE, GL_TRUE },  // udVDF_UByte4N_RGBA
  { GL_BGRA,  GL_UNSIGNED_BYTE, GL_TRUE },  // udVDF_UByte4N_BGRA
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
// Author: Manu Evans, May 2015
void udGPU_RenderVertices(udShaderProgram *pProgram, udFormatDeclaration *pVertexDecl, udArrayBuffer *pVB[], udPrimitiveType primType, size_t vertexCount, size_t firstVertex)
{
  udVertexRange r;
  r.firstVertex = (uint32_t)firstVertex;
  r.vertexCount = (uint32_t)vertexCount;
  udGPU_RenderRanges(pProgram, pVertexDecl, pVB, primType, &r, 1);
}

// ***************************************************************************************
// Author: Manu Evans, Aug 2015
void udGPU_RenderIndices(udShaderProgram *pProgram, udFormatDeclaration *pVertexDecl, udArrayBuffer *pVB[], udArrayBuffer *pIB, udPrimitiveType primType, size_t indexCount, size_t udUnusedParam(firstIndex), size_t udUnusedParam(firstVertex))
{
  udArrayElement *pElements = pVertexDecl->pElements;
  udArrayElementData *pElementData = pVertexDecl->pElementData;

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

    udVertexDataFormatGL &f = s_dataFormat[pElements[a].format];
    glVertexAttribPointer(attribs[a], f.components, f.type, f.normalise, pElementData[a].stride, (GLvoid*)(size_t)pElementData[a].offset);
    glEnableVertexAttribArray(attribs[a]);
  }

  // issue the draw call
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pIB->buffer);
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
      type = GL_UNSIGNED_SHORT;
      UDASSERT(false, "Invalid index buffer type!");
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
void udGPU_RenderRanges(udShaderProgram *pProgram, udFormatDeclaration *pVertexDecl, udArrayBuffer *pVB[], udPrimitiveType primType, udVertexRange *pRanges, size_t rangeCount, PrimCallback *pCallback, void *pCallbackData)
{
  udArrayElement *pElements = pVertexDecl->pElements;
  udArrayElementData *pElementData = pVertexDecl->pElementData;

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

    udVertexDataFormatGL &f = s_dataFormat[pElements[a].format];
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
void udGPU_Init()
{
#if defined(USE_GLEW)
  glewInit();
#endif
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
void udGPU_Deinit()
{
}

#endif // UDRENDER_DRIVER == UDDRIVER_OPENGL
