#pragma once
#ifndef EPIMAGE_H
#define EPIMAGE_H

enum epImageFormat
{
  epIF_Unknown = -1,

  epIF_RGBA8 = 0,
  epIF_BGRA8,
  epIF_RGB8,
  epIF_BGR8,

  epIF_Max
};

struct epImageSurface
{
  epImageFormat format;
  uint32_t width, height, depth;
  void *pImage;
};

struct epImage
{
  size_t elements, mips;
  epImageSurface *pSurfaces; // elements*mips surfaces

  struct KVP
  {
    const char *pKey, *pValue;
  };
  size_t numMetadataEntries;
  KVP *pMetadata;
};

epImage* epImage_ReadImage(void *pBuffer, size_t bufferLen, const char *pFileExt);
void* epImage_WriteImage(epImage *pImage, const char *pFileExt, size_t *pOutputSize);

#endif // EPIMAGE_H
