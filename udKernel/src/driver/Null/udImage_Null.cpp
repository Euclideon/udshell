#include "hal/driver.h"

#if UDIMAGE_DRIVER == UDDRIVER_NULL

#include "hal/image.h"

void udImage_InitInternal()
{
}

void udImage_DeinitInternal()
{
}

udImage* udImage_ReadImage(void *pBuffer, size_t bufferLen, const char *pFileExt)
{
  return nullptr;
}

void* udImage_WriteImage(udImage *pImage, const char *pFileExt, size_t *pOutputSize)
{
  *pOutputSize = 0;
  return nullptr;
}

#endif
