#include "driver.h"

#if EPIMAGE_DRIVER == EPDRIVER_QT

#include "hal/image.h"

#include <QImage>
#include <QImageReader>
#include <QBuffer>

void epImage_InitInternal()
{

}

void epImage_DeinitInternal()
{

}

epImageFormat ConvertQTImageFormatToEP(QImage::Format imageFormat)
{
  switch (imageFormat)
  {
  case QImage::Format_RGB32:
  case QImage::Format_ARGB32:
    return epIF_BGRA8;
  case QImage::Format_RGBX8888:
  case QImage::Format_RGBA8888:
    return epIF_RGBA8;
  case QImage::Format_RGB888:
    return epIF_RGB8;
  case QImage::Format_Invalid:
    return epIF_Unknown;

  // TODO: add support for more formats
  case QImage::Format_ARGB32_Premultiplied:
  case QImage::Format_RGB16:
  case QImage::Format_ARGB8565_Premultiplied:
  case QImage::Format_RGB666:
  case QImage::Format_ARGB6666_Premultiplied:
  case QImage::Format_RGB555:
  case QImage::Format_ARGB8555_Premultiplied:
  case QImage::Format_RGB444:
  case QImage::Format_ARGB4444_Premultiplied:
  case QImage::Format_Mono:
  case QImage::Format_MonoLSB:
  case QImage::Format_Indexed8:
  case QImage::Format_RGBA8888_Premultiplied:
  case QImage::Format_BGR30:
  case QImage::Format_A2BGR30_Premultiplied:
  case QImage::Format_RGB30:
  case QImage::Format_A2RGB30_Premultiplied:
  case QImage::NImageFormats:
  case QImage::Format_Alpha8:
  case QImage::Format_Grayscale8:
  default:
    return epIF_Unknown;
  }
}

epImage* epImage_LoadImage(void *pBuffer, size_t bufferLen, const char *)
{
  QByteArray a = QByteArray::fromRawData((const char *)(pBuffer), (int)bufferLen);
  QBuffer b;
  b.setData(a);
  b.open(QIODevice::ReadOnly);
  QImageReader qImageReader(&b);
  QImage qImage(qImageReader.read());

  if (qImage.isNull())
  {
    epDebugPrintf("Error loading image -- %s\n", qImageReader.errorString().toUtf8().data());
    return nullptr;
  }

  epImage *pOutput = (epImage*)epAlloc(sizeof(epImage) + sizeof(epImageSurface));
  if (!pOutput)
  {
    epDebugPrintf("Error allocating epImage\n");
    return nullptr;
  }

  pOutput->pSurfaces = (epImageSurface*)&pOutput[1];
  pOutput->elements = 1;
  pOutput->mips = 1;

  pOutput->numMetadataEntries = 0;
  pOutput->pMetadata = nullptr;

  epImageFormat format = ConvertQTImageFormatToEP(qImage.format());
  QImage convertedImage;
  QImage &resultImage = format == epIF_Unknown ? convertedImage : qImage;

  if (format == epIF_Unknown)
  {
    // TODO: ** Instead of converting we should load the image in it's raw format
    // HACK: we convert to ARGB32 for now
    convertedImage = qImage.convertToFormat(QImage::Format_ARGB32, Qt::ThresholdDither); // No dithering
    if (convertedImage.isNull())
    {
      epFree(pOutput);
      epDebugPrintf("Error converting image to BGRA32 format\n");
      return nullptr;
    }

    format = epIF_BGRA8;
  }

  epImageSurface &surface = pOutput->pSurfaces[0];

  surface.width = resultImage.width();
  surface.height = resultImage.height();
  surface.depth = 0;

  surface.format = format;

  surface.pImage = epAlloc(surface.width * surface.height * 4);
  if (!surface.pImage)
  {
    epFree(pOutput);
    epDebugPrintf("Error allocating epImage surface\n");
    return nullptr;
  }

  const uchar *qImageBuffer = resultImage.bits();
  memcpy(surface.pImage, qImageBuffer, surface.width * surface.height * 4);

  return pOutput;
}

void epImage_DestroyImage(epImage **ppImage)
{
  if (ppImage && *ppImage)
  {
    for (size_t i = 0; i < (*ppImage)->elements; ++i)
      epFree((*ppImage)->pSurfaces[i].pImage);
    epFree(*ppImage);
    *ppImage = nullptr;
  }
}

void* epImage_WriteImage(epImage epUnusedParam(*pImage), const char epUnusedParam(*pFileExt), size_t epUnusedParam(*pOutputSize))
{
  return nullptr;
}

#else
EPEMPTYFILE
#endif
