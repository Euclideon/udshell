#include "udViewerPlatform.h"

#if UDRENDER_DRIVER == UDDRIVER_OPENGL

#include "udGPU.h"
#include "udShader.h"
#include "udVertex.h"

#include "udOpenGL_Internal.h"


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
  { 4,        GL_FLOAT,         GL_FALSE }, // udVDF_Float4,
  { 3,        GL_FLOAT,         GL_FALSE }, // udVDF_Float3,
  { 2,        GL_FLOAT,         GL_FALSE }, // udVDF_Float2,
  { 1,        GL_FLOAT,         GL_FALSE }, // udVDF_Float1,
  { 4,        GL_UNSIGNED_BYTE, GL_TRUE },  // udVDF_UByte4N_RGBA,
  { GL_BGRA,  GL_UNSIGNED_BYTE, GL_TRUE },  // udVDF_UByte4N_BGRA,
};

char *s_attribNames[] =
{
  "a_position\0",
  "a_normal\0",
  "a_colour\0",
  "a_texcoord\0"
};
const int s_attribNameLen[] = // HAX: this length is used to overwrite the '\0' with an index at runtime
{
  9,
  7,
  7,
  9
};


// ***************************************************************************************
// Author: Manu Evans, May 2015
void udGPU_RenderVertices(udShaderProgram *pProgram, udVertexBuffer *pVB, udPrimitiveType primType, size_t numVertices)
{
  udVertexElement *pElements = pVB->pVertexDeclaration->pElements;
  udVertexElementData *pElementData = pVB->pVertexDeclaration->pElementData;

  // bind the program
  glBindBuffer(GL_ARRAY_BUFFER, pVB->vb);

  // bind the vertex streams to the shader attributes
  GLint attribs[16];
  for(int a=0; a<pVB->pVertexDeclaration->numElements; ++a)
  {
    udVertexElementType type = pElements[a].type;

    // MAD HAX: we lookup the attributes by name each render
    //          this is thoroughly lame, and should be cached on shader creation
    if(pElements[a].index == 0)
      attribs[a] = glGetAttribLocation(pProgram->program, s_attribNames[type]);
    if(pElements[a].index > 0 || attribs[a] == -1)
    {
      // MAD HAX: we reserved an extra '\0' on the end of each attrib name
      //          we'll overwrite that with the stream index temporarily...
      s_attribNames[type][s_attribNameLen[type]] = (char)('0' + pElements[a].index);
      attribs[a] = glGetAttribLocation(pProgram->program, s_attribNames[type]);
      s_attribNames[type][s_attribNameLen[type]] = 0;
    }

    if(attribs[a] == -1)
      continue;

    udVertexDataFormatGL &f = s_DataFormat[pElements[a].format];
    glVertexAttribPointer(attribs[a], f.components, f.type, f.normalise, pElementData[a].stride, (GLvoid*)(size_t)pElementData[a].offset);
    glEnableVertexAttribArray(attribs[a]);
  }

  // issue the draw call
  glDrawArrays(s_PrimTypes[primType], 0, (GLsizei)numVertices);

  // unbind the attributes  TODO: perhaps we can remove this...?
  for(int a=0; a<pVB->pVertexDeclaration->numElements; ++a)
  {
    if(attribs[a] != -1)
      glDisableVertexAttribArray(attribs[a]);
  }
}


// ***************************************************************************************
// Author: Manu Evans, May 2015
void udGPU_Init()
{
  glewInit();
}

#endif // UDRENDER_DRIVER == UDDRIVER_OPENGL
