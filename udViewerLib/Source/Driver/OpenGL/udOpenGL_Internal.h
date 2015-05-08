#pragma once
#if !defined(UD_OPENGL)
#define UD_OPENGL

#include "udViewerPlatform.h"

#if UDWINDOW_DRIVER == UDDRIVER_GLUT
# include "GL/glew.h"
//# include "GL/wglew.h"
#else
# if defined(USE_GLES)
#   include <GLES2/gl2ext.h>
# else
#   include <GL/gl.h>
#   include <GL/glext.h>
# endif
#endif

#include "udTexture.h"
#include "udVertex.h"

struct udTexture
{
  udTextureType type;
  udImageFormat format;
  size_t width, height, depth;
  size_t elements;
  int levels;
  GLuint texture;
  void *pMappedBuffer;
};

struct udGLTextureFormat
{
  GLint internalFormat;
  GLenum format;
  GLenum type;
};



struct udVertexElementData
{
  int offset;
  int stride;
};

struct udVertexDeclaration
{
  udVertexElement *pElements;
  udVertexElementData *pElementData;
  int numElements;
};

struct udVertexBuffer
{
  udVertexDeclaration *pVertexDeclaration;

  GLuint vb;
  size_t bufferLen;
  void *pMappedBuffer;
};


struct udShader
{
  GLuint shader;
};

struct udShaderProgram
{
  GLuint program;
};

#endif // UD_OPENGL
