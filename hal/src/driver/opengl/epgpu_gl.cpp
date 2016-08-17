#include "driver.h"

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
// Author: Manu Evans, Nov 2015
void epGPU_Clear(uint32_t clearBits, float *pColor, float depth, int stencil)
{
  GLbitfield mask = 0;
  if (clearBits & epC_Color)
  {
    mask |= GL_COLOR_BUFFER_BIT;
    glClearColor(pColor[0], pColor[1], pColor[2], pColor[3]);
  }
  if (clearBits & epC_Depth)
  {
    mask |= GL_DEPTH_BUFFER_BIT;
    glClearDepthf(depth);
  }
  if (clearBits & epC_Stencil)
  {
    mask |= GL_STENCIL_BUFFER_BIT;
    glClearStencil(stencil);
  }
  glClear(mask);
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
void epGPU_RenderVertices(epShaderProgram *pProgram, epShaderInputConfig *pConfig, epArrayBuffer *pVB[], epPrimitiveType primType, size_t vertexCount, size_t firstVertex)
{
  epVertexRange r;
  r.firstVertex = (uint32_t)firstVertex;
  r.vertexCount = (uint32_t)vertexCount;
  epGPU_RenderRanges(pProgram, pConfig, pVB, primType, &r, 1);
}

// ***************************************************************************************
// Author: Manu Evans, Aug 2015
void epGPU_RenderIndices(epShaderProgram *epUnusedParam(pProgram), epShaderInputConfig *pConfig, epArrayBuffer *pVB[], epArrayBuffer *pIB, epPrimitiveType primType, size_t indexCount, size_t epUnusedParam(firstIndex), size_t epUnusedParam(firstVertex))
{
  epArrayElement *pElements = pConfig->pElements;
  epArrayElementData *pElementData = pConfig->pElementData;

  // bind the vertex streams to the shader attributes
  bool boundVB[16] = { false };
  for (int i = 0; i < pConfig->numElements; ++i)
  {
    if (!boundVB[pElements[i].stream])
    {
      // bind the buffer
      glBindBuffer(GL_ARRAY_BUFFER, pVB[pElements[i].stream]->buffer);
      boundVB[pElements[i].stream] = true;
    }

    epVertexDataFormatGL &f = s_dataFormat[pElements[i].format];
    glVertexAttribPointer(pElementData[i].attribLocation, f.components, f.type, f.normalise, pElements[i].stride, (GLvoid*)(size_t)pElements[i].offset);
    glEnableVertexAttribArray(pElementData[i].attribLocation);
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

  for (int i = 0; i < pConfig->numElements; ++i)
    glDisableVertexAttribArray(pElementData[i].attribLocation);
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
void epGPU_RenderRanges(epShaderProgram *epUnusedParam(pProgram), epShaderInputConfig *pConfig, epArrayBuffer *pVB[], epPrimitiveType primType, epVertexRange *pRanges, size_t rangeCount, PrimCallback *pCallback, void *pCallbackData)
{
  epArrayElement *pElements = pConfig->pElements;
  epArrayElementData *pElementData = pConfig->pElementData;

  // bind the vertex streams to the shader attributes
  bool boundVB[16] = { false };
  for (int i = 0; i < pConfig->numElements; ++i)
  {
    if (!boundVB[pElements[i].stream])
    {
      // bind the buffer
      glBindBuffer(GL_ARRAY_BUFFER, pVB[pElements[i].stream]->buffer);
      boundVB[pElements[i].stream] = true;
    }

    epVertexDataFormatGL &f = s_dataFormat[pElements[i].format];
    glVertexAttribPointer(pElementData[i].attribLocation, f.components, f.type, f.normalise, pElements[i].stride, (GLvoid*)(size_t)pElements[i].offset);
    glEnableVertexAttribArray(pElementData[i].attribLocation);
  }

  // issue the draw call
  for (size_t i=0; i<rangeCount; ++i)
  {
    if (pCallback)
      pCallback(i, pCallbackData);
    glDrawArrays(s_primTypes[primType], (GLint)pRanges[i].firstVertex, (GLsizei)pRanges[i].vertexCount);
  }

  for (int i = 0; i < pConfig->numElements; ++i)
    glDisableVertexAttribArray(pElementData[i].attribLocation);
}

// ***************************************************************************************
epSyncPoint *epGPU_CreateSyncPoint()
{
  // TODO: Implement this!
  return nullptr;
}

// ***************************************************************************************
void epGPU_WaitSync(epSyncPoint **ppSync)
{
  epUnused(ppSync);
  // TODO: Implement this!
}

// ***************************************************************************************
void epGPU_DestroySyncPoint(epSyncPoint **ppSync)
{
  epUnused(ppSync);
  // TODO: Implement this!
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

// ***************************************************************************************
// Author: David Ely, August 2016
void epRenderState_SetDepthCompare(bool enable, epCompareFunc func)
{
  if (enable)
  {
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_NEVER + (GLenum)func);
  }
  else
  {
    glDisable(GL_DEPTH_TEST);
  }
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
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CW);
    glCullFace(s_lookup[mode]);
  }
  else
  {
    glDisable(GL_CULL_FACE);
  }
}

// ***************************************************************************************
// Author: David Ely, August 2016
void epRenderState_SetBlendMode(bool enable, epBlendMode mode)
{
  if (enable)
  {
    glEnable(GL_BLEND);
    if (mode == epBM_Alpha)
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    else
      glBlendFunc(GL_ONE, GL_ONE);
  }
  else
  {
    glDisable(GL_BLEND);
  }
}

// ***************************************************************************************
// Author: David Ely, August 2016
void epRenderState_SetColorMask(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
  glColorMask(r, g, b, a);
}

// ***************************************************************************************
// Author: David Ely, August 2016
void epRenderState_SetDepthMask(bool flag)
{
  glDepthMask(flag ? GL_TRUE : GL_FALSE);
}


#else
EPEMPTYFILE
#endif // EPRENDER_DRIVER == EPDRIVER_OPENGL

