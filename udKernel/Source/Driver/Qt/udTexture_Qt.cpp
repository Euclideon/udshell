#include "udDriver.h"

#if UDRENDER_DRIVER == UDDRIVER_QT

#include "udTexture.h"
#include "udQt_Internal.h"

#include <QOpenGLTexture>


struct udQtGLFormats
{
  QOpenGLTexture::TextureFormat internalFormat;
  QOpenGLTexture::PixelFormat format;
  QOpenGLTexture::PixelType type;
};


static QOpenGLTexture::Target s_textureType[udTT_Max] =
{
  QOpenGLTexture::Target1D,
  QOpenGLTexture::Target1DArray,
  QOpenGLTexture::Target2D,
  QOpenGLTexture::Target2DArray,
  QOpenGLTexture::TargetCubeMap,
  QOpenGLTexture::TargetCubeMapArray,
  QOpenGLTexture::Target3D
};

udQtGLFormats s_GLFormats[udIF_Max] =
{
  { QOpenGLTexture::RGBA8_UNorm, QOpenGLTexture::RGBA, QOpenGLTexture::UInt32_RGBA8_Rev },    // udIF_RGBA8
  { QOpenGLTexture::RGBA8_UNorm, QOpenGLTexture::BGRA, QOpenGLTexture::UInt32_RGBA8_Rev },    // udIF_BGRA8
  //{ QOpenGLTexture::LuminanceFormat, QOpenGLTexture::Luminance, QOpenGLTexture::Float32 },  // udIF_R_F32
  // TODO: does this work with older gl versions?
  { QOpenGLTexture::R32F, QOpenGLTexture::Red, QOpenGLTexture::Float32 },                     // udIF_R_F32
};


// ***************************************************************************************
udTexture *udTexture_CreateTexture(udTextureType type, size_t width, size_t height, int levels, udImageFormat format)
{
  bool result = true;
  udTexture *pTex = nullptr;

  QOpenGLTexture *pQtTexture = new QOpenGLTexture(s_textureType[type]);
  UD_ERROR_IF(!pQtTexture->create(), false);
  pQtTexture->bind();
  pQtTexture->setMinMagFilters(QOpenGLTexture::Linear, QOpenGLTexture::Linear);
  pQtTexture->setWrapMode(QOpenGLTexture::ClampToEdge);

  pTex = udAllocType(udTexture, 1, udAF_None);

  pTex->type = type;
  pTex->format = format;
  pTex->width = width;
  pTex->height = height;
  pTex->depth = 0;
  pTex->levels = levels;
  pTex->elements = 1;
  pTex->pTexture = pQtTexture;

epilogue:
  if (!result)
  {
    udDebugPrintf("Error creating texture\n");
    delete pQtTexture;
  }
  return pTex;
}

// ***************************************************************************************
void udTexture_DestroyTexture(udTexture **ppTex)
{
  QOpenGLTexture *pQtTexture = (*ppTex)->pTexture;
  UDASSERT(pQtTexture, "QOpenGLTexture object has not been created");

  pQtTexture->release();
  pQtTexture->destroy();
  delete pQtTexture;

  udFree(*ppTex);
  *ppTex = nullptr;
}

// ***************************************************************************************
void udTexture_SetImageData(udTexture *pTex, int element, int level, void *pImage)
{
  // TODO: should we use the function pointer instead? maybe api check?

  QOpenGLTexture *pQtTexture = pTex->pTexture;

  // TODO: remove these checks once we are confident in udKernel and the Qt driver
  UDASSERT(pQtTexture, "QOpenGLTexture object has not been created");
  UDASSERT(pQtTexture->isCreated(), "GL Texture has not been created");
  UDASSERT(!pQtTexture->isStorageAllocated(), "Texture storage has already been allocated");
  UDASSERT(pTex->type == udTT_2D, "Qt currently driver currently only supports udTT_2D type textures");

  udQtGLFormats &format = s_GLFormats[pTex->format];
  pQtTexture->bind();

  int elementWidth = static_cast<int>(pTex->width >> level);
  int elementHeight = static_cast<int>(pTex->height >> level);
  //int elementDepth = static_cast<int>(pTex->depth >> level);

  // TODO: move these to udTexture_CreateTexture ?
  pQtTexture->setFormat(format.internalFormat);
  pQtTexture->setMipLevels(pTex->levels);
  pQtTexture->setSize(elementWidth, elementHeight);

  pQtTexture->allocateStorage();
  if (!pQtTexture->isStorageAllocated())
  {
    // TODO: handle this error?
    udDebugPrintf("Error allocating texture storage\n");
    return;
  }

  pQtTexture->setData(level, format.format, format.type, pImage);
}

#endif // UDRENDER_DRIVER == UDDRIVER_QT
