#pragma once
#ifndef UDIMAGE_H
#define UDIMAGE_H

enum udImageFormat
{
  udIF_Unknown = -1,

  udIF_RGBA8 = 0,
  udIF_BGRA8,
  udIF_RGB8,
  udIF_BGR8,

  udIF_Max
};

struct udImageSurface
{
  udImageFormat format;
  uint32_t width, height, depth;
  void *pImage;
};

struct udImage
{
  size_t elements, mips;
  udImageSurface *pSurfaces; // elements*mips surfaces

  struct KVP
  {
    const char *pKey, *pValue;
  };
  size_t numMetadataEntries;
  KVP *pMetadata;
};

udImage* udImage_ReadImage(void *pBuffer, size_t bufferLen, const char *pFileExt);
void* udImage_WriteImage(udImage *pImage, const char *pFileExt, size_t *pOutputSize);

#endif // UDIMAGE_H
