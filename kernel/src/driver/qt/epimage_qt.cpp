#include "hal/driver.h"

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

  // TODO: ** Instead of converting we should load the image in it's raw format
  // HACK: we convert to ARGB32 for now
  QImage qcImage = qImage.convertToFormat(QImage::Format_ARGB32, Qt::ThresholdDither); // No dithering
  if (qcImage.isNull())
  {
    epFree(pOutput);
    epDebugPrintf("Error converting image to BGRA32 format\n");
    return nullptr;
  }

  epImageSurface &surface = pOutput->pSurfaces[0];

  surface.width = qcImage.width();
  surface.height = qcImage.height();
  surface.depth = 0;

  surface.format = epIF_BGRA8;

  surface.pImage = epAlloc(surface.width * surface.height * 4);
  if (!surface.pImage)
  {
    epFree(pOutput);
    epDebugPrintf("Error allocating epImage surface\n");
    return nullptr;
  }

  const uchar *qImageBuffer = qcImage.bits();
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
