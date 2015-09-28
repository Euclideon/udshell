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
#elif EPWINDOW_DRIVER == EPDRIVER_GLUT
# define FREEGLUT_STATIC
# define FREEGLUT_LIB_PRAGMAS 0
# define FREEGLUT_BUILDING_LIB
# include "GL/freeglut.h"
# include "GL/glext.h"
#elif EPWINDOW_DRIVER == EPDRIVER_SDL
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

enum epArrayType
{
  epAT_VertexArray,
  epAT_IndexArray,
};

struct epTexture
{
  epTextureType type;
  epImageFormat format;
  size_t width, height, depth;
  size_t elements;
  int levels;
  GLuint texture;
  void *pMappedBuffer;
};

struct epGLTextureFormat
{
  GLint internalFormat;
  GLenum format;
  GLenum type;
};

struct epArrayElementData
{
  int offset;
  int stride;
};

struct epFormatDeclaration
{
  epArrayElement *pElements;
  epArrayElementData *pElementData;
  int numElements;
};

struct epArrayBuffer
{
  epArrayType type;

  GLuint buffer;
  size_t bufferLen;
  void *pMappedBuffer;

  epArrayDataFormat *pFormat;
  size_t numElements;
};

struct epShader
{
  GLuint shader;
};

struct epShaderProgram
{
  GLuint program;

  struct Param
  {
    char *pName;
    size_t type;
  };

  size_t numAttributes;
  Param *pAttributes;

  size_t numUniforms;
  Param *pUniforms;
};

#endif // UD_OPENGL
