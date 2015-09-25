#include "hal/driver.h"

#if EPIMAGE_DRIVER == EPDRIVER_NULL

#include "hal/image.h"

void epImage_InitInternal()
{
}

void epImage_DeinitInternal()
{
}

epImage* epImage_ReadImage(void *pBuffer, size_t bufferLen, const char *pFileExt)
{
  return nullptr;
}

void* epImage_WriteImage(epImage *pImage, const char *pFileExt, size_t *pOutputSize)
{
  *pOutputSize = 0;
  return nullptr;
}

#endif
