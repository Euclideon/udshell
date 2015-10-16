#include "hal/driver.h"

#if EPRENDER_DRIVER == EPDRIVER_QT

#include "hal/texture.h"

#include "eprender_qt.h"

#include <QOpenGLTexture>


struct epQtGLFormats
{
  QOpenGLTexture::TextureFormat internalFormat;
  QOpenGLTexture::PixelFormat format;
  QOpenGLTexture::PixelType type;
};


static QOpenGLTexture::Target s_textureType[epTT_Max] =
{
  QOpenGLTexture::Target1D,
  QOpenGLTexture::Target1DArray,
  QOpenGLTexture::Target2D,
  QOpenGLTexture::Target2DArray,
  QOpenGLTexture::TargetCubeMap,
  QOpenGLTexture::TargetCubeMapArray,
  QOpenGLTexture::Target3D
};

epQtGLFormats s_GLFormats[epIF_Max] =
{
  { QOpenGLTexture::RGBA8_UNorm, QOpenGLTexture::RGBA, QOpenGLTexture::UInt32_RGBA8_Rev },    // epIF_RGBA8
  { QOpenGLTexture::RGBA8_UNorm, QOpenGLTexture::BGRA, QOpenGLTexture::UInt32_RGBA8_Rev },    // epIF_BGRA8
  //{ QOpenGLTexture::LuminanceFormat, QOpenGLTexture::Luminance, QOpenGLTexture::Float32 },  // epIF_R_F32
  // TODO: does this work with older gl versions?
  { QOpenGLTexture::R32F, QOpenGLTexture::Red, QOpenGLTexture::Float32 },                     // epIF_R_F32
};


// ***************************************************************************************
epTexture *epTexture_CreateTexture(epTextureType type, size_t width, size_t height, int levels, epImageFormat format)
{
  bool result = true;
  epTexture *pTex = nullptr;

  QOpenGLTexture *pQtTexture = new QOpenGLTexture(s_textureType[type]);
  EP_ERROR_IF(!pQtTexture->create(), false);
  pQtTexture->bind();
  pQtTexture->setMinMagFilters(QOpenGLTexture::Linear, QOpenGLTexture::Linear);
  pQtTexture->setWrapMode(QOpenGLTexture::ClampToEdge);

  pTex = epAllocType(epTexture, 1, epAF_None);

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
    epDebugPrintf("Error creating texture\n");
    delete pQtTexture;
  }
  return pTex;
}

// ***************************************************************************************
void epTexture_DestroyTexture(epTexture **ppTex)
{
  QOpenGLTexture *pQtTexture = (*ppTex)->pTexture;
  EPASSERT(pQtTexture, "QOpenGLTexture object has not been created");

  pQtTexture->release();
  pQtTexture->destroy();
  delete pQtTexture;

  epFree(*ppTex);
  *ppTex = nullptr;
}

// ***************************************************************************************
void epTexture_SetImageData(epTexture *pTex, int epUnusedParam(element), int level, void *pImage)
{
  // TODO: should we use the function pointer instead? maybe api check?

  QOpenGLTexture *pQtTexture = pTex->pTexture;

  // TODO: remove these checks once we are confident in Kernel and the Qt driver
  EPASSERT(pQtTexture, "QOpenGLTexture object has not been created");
  EPASSERT(pQtTexture->isCreated(), "GL Texture has not been created");
  EPASSERT(!pQtTexture->isStorageAllocated(), "Texture storage has already been allocated");
  EPASSERT(pTex->type == epTT_2D, "Qt currently driver currently only supports epTT_2D type textures");

  epQtGLFormats &format = s_GLFormats[pTex->format];
  pQtTexture->bind();

  int elementWidth = static_cast<int>(pTex->width >> level);
  int elementHeight = static_cast<int>(pTex->height >> level);
  //int elementDepth = static_cast<int>(pTex->depth >> level);

  // TODO: move these to epTexture_CreateTexture ?
  pQtTexture->setFormat(format.internalFormat);
  pQtTexture->setMipLevels(pTex->levels);
  pQtTexture->setSize(elementWidth, elementHeight);

  pQtTexture->allocateStorage();
  if (!pQtTexture->isStorageAllocated())
  {
    // TODO: handle this error?
    epDebugPrintf("Error allocating texture storage\n");
    return;
  }

  pQtTexture->setData(level, format.format, format.type, pImage);
}

#else
EPEMPTYFILE
#endif // EPRENDER_DRIVER == EPDRIVER_QT
