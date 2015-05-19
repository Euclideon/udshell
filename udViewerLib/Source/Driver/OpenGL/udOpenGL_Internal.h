#pragma once
#if !defined(UD_OPENGL)
#define UD_OPENGL

#include "udViewerPlatform.h"

#if UDWINDOW_DRIVER == UDDRIVER_GLUT
// ** Uses GLEW instead
//# define FREEGLUT_STATIC
//# define FREEGLUT_LIB_PRAGMAS 0
//# define FREEGLUT_BUILDING_LIB
//# include "GL/freeglut.h"
//# include "GL/glext.h"
#elif UDWINDOW_DRIVER == UDDRIVER_SDL
// ** Uses GLEW instead
//# include <SDL2/SDL_opengl.h>
#else
# if defined(USE_GLES)
#   include <GLES2/gl2.h>
#   include <GLES2/gl2ext.h>
# else
#   include <GL/gl.h>
#   include <GL/glext.h>
# endif
#endif
#if defined(USE_GLEW)
# include "GL/glew.h"
//# include "GL/wglew.h"
#endif

#if !defined(GL_BGRA)
# define GL_BGRA 0x80E1
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
