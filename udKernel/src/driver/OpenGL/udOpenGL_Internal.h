#pragma once
#if !defined(UD_OPENGL)
#define UD_OPENGL

#include "hal/driver.h"

#if defined(USE_GLEW)
# include "GL/glew.h"
//# include "GL/wglew.h"
#elif defined(USE_GLES)
# include <GLES2/gl2.h>
# include <GLES2/gl2ext.h>
#elif UDWINDOW_DRIVER == UDDRIVER_GLUT
# define FREEGLUT_STATIC
# define FREEGLUT_LIB_PRAGMAS 0
# define FREEGLUT_BUILDING_LIB
# include "GL/freeglut.h"
# include "GL/glext.h"
#elif UDWINDOW_DRIVER == UDDRIVER_SDL
# include <SDL2/SDL_opengl.h>
#else
# include <GL/gl.h>
# include <GL/glext.h>
#endif

#if !defined(GL_BGRA)
# define GL_BGRA 0x80E1
#endif

#if UD_DEBUG
# define udCheckGLError() { GLenum err = glGetError(); if(err != GL_NO_ERROR) udDebugPrintf("%s(%d): GL error %04X", __FILE__, __LINE__, err); }
#else
# define udCheckGLError()
#endif

#include "hal/texture.h"
#include "hal/vertex.h"

enum udArrayType
{
  udAT_VertexArray,
  udAT_IndexArray,
};

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

struct udArrayElementData
{
  int offset;
  int stride;
};

struct udFormatDeclaration
{
  udArrayElement *pElements;
  udArrayElementData *pElementData;
  int numElements;
};

struct udArrayBuffer
{
  udArrayType type;

  GLuint buffer;
  size_t bufferLen;
  void *pMappedBuffer;

  udArrayDataFormat *pFormat;
  size_t numElements;
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
