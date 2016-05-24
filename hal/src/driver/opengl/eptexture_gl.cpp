#include "driver.h"

#if EPRENDER_DRIVER == EPDRIVER_OPENGL

#include "hal/shader.h"
#include "hal/texture.h"

#include "ep_opengl.h"


GLenum s_textureType[epTT_Max] =
{
#if !defined(USE_GLES)
  GL_TEXTURE_1D,
  GL_TEXTURE_1D_ARRAY,
  GL_TEXTURE_2D,
  GL_TEXTURE_2D_ARRAY,
  GL_TEXTURE_CUBE_MAP,
  GL_TEXTURE_CUBE_MAP_ARRAY,
  GL_TEXTURE_3D
#else
  0,
  0,
  GL_TEXTURE_2D,
  0,
  GL_TEXTURE_CUBE_MAP,
  0,
  0
#endif
};

static GLuint s_cubeFace[6] =
{
  GL_TEXTURE_CUBE_MAP_POSITIVE_X,
  GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
  GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
  GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
  GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
  GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
};

#if !defined(USE_GLES)
epGLTextureFormat s_GLFormats[epIF_Max] =
{
  { GL_RGBA8, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV },   // epIF_RGBA8
  { GL_RGBA8, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV },   // epIF_BGRA8
  { GL_LUMINANCE, GL_LUMINANCE, GL_FLOAT },             // epIF_R_F32
};
#else
epGLTextureFormat s_GLFormats[epIF_Max] =
{
  { GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE },               // epIF_RGBA8
  { GL_RGBA, GL_BGRA, GL_UNSIGNED_BYTE },               // epIF_BGRA8
  { GL_LUMINANCE, GL_LUMINANCE, GL_FLOAT },             // epIF_R_F32
};
#endif


// ***************************************************************************************
// Author: Manu Evans, May 2015
epTexture* epTexture_CreateTexture(epTextureType type, size_t width, size_t height, int levels, epImageFormat format)
{
  epTexture *pTex = (epTexture*)epAlloc(sizeof(epTexture));

  pTex->type = type;
  pTex->format = format;
  pTex->width = width;
  pTex->height = height;
  pTex->depth = 0;
  pTex->levels = levels;
  pTex->elements = 1;

  GLuint t = s_textureType[pTex->type];

  glGenTextures(1, &pTex->texture);
  glBindTexture(t, pTex->texture);
  glTexParameteri(t, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(t, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(t, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(t, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
#if !defined(USE_GLES)
  if(pTex->type == epTT_3D)
    glTexParameteri(t, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
#endif

  return pTex;
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
void epTexture_DestroyTexture(epTexture **ppTex)
{
  glDeleteTextures(1, &(*ppTex)->texture);
  epFree(*ppTex);
  *ppTex = nullptr;
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
void epTexture_SetImageData(epTexture *pTex, int element, int level, const void *pImage)
{
  epGLTextureFormat &format = s_GLFormats[pTex->format];
  GLuint type = s_textureType[pTex->type];

  GLsizei elementWidth = GLsizei(pTex->width >> level);
  GLsizei elementHeight = GLsizei(pTex->height >> level);
  GLsizei elementDepth = GLsizei(pTex->depth >> level);

  glBindTexture(type, pTex->texture);

  // texture arrays
  switch(pTex->type)
  {
    case epTT_1D:
#if !defined(USE_GLES)
      glTexImage1D(type, level, format.internalFormat, elementWidth, 0, format.format, format.type, pImage);
#else
      EPASSERT(false, "Not supported in GLES2!");
#endif
      break;
    case epTT_1DArray:
      if(element == -1)
        glTexImage2D(type, level, format.internalFormat, elementWidth, (GLsizei)pTex->elements, 0, format.format, format.type, pImage);
      else
        glTexSubImage2D(type, level, 0, element, elementWidth, 1, format.format, format.type, pImage);
      break;
    case epTT_2D:
      glTexImage2D(type, level, format.internalFormat, elementWidth, elementHeight, 0, format.format, format.type, pImage);
      break;
    case epTT_2DArray:
#if !defined(USE_GLES)
      if(element == -1)
        glTexImage3D(type, level, format.internalFormat, elementWidth, elementHeight, (GLsizei)pTex->elements, 0, format.format, format.type, pImage);
      else
        glTexSubImage3D(type, level, 0, 0, element, elementWidth, elementHeight, 1, format.format, format.type, pImage);
#else
      EPASSERT(false, "Not supported in GLES2!");
#endif
      break;
    case epTT_Cube:
      glTexImage2D(s_cubeFace[element], level, format.internalFormat, elementWidth, elementHeight, 0, format.format, format.type, pImage);
      break;
    case epTT_CubeArray:
      EPASSERT(false, "TODO: cubemap arrays!");
      break;
    case epTT_3D:
#if !defined(USE_GLES)
      glTexImage3D(type, level, format.internalFormat, elementWidth, elementHeight, elementDepth, 0, format.format, format.type, pImage);
#else
      EPASSERT(false, "Not supported in GLES2!");
#endif
      break;
    default:
      EPUNREACHABLE;
  }
}

#else
EPEMPTYFILE
#endif // EPRENDER_DRIVER == EPDRIVER_OPENGL
