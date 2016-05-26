#include "driver.h"

#if EPIMAGE_DRIVER == EPDRIVER_NULL

#include "hal/image.h"

void epImage_InitInternal()
{
}

void epImage_DeinitInternal()
{
}

epImage* epImage_LoadImage(void *epUnusedParam(pBuffer), size_t epUnusedParam(bufferLen), const char *epUnusedParam(pFileExt))
{
  return nullptr;
}

void epImage_DestroyImage(epImage **epUnusedParam(ppImage))
{
}

void* epImage_WriteImage(epImage *epUnusedParam(pImage), const char *epUnusedParam(pFileExt), size_t *pOutputSize)
{
  *pOutputSize = 0;
  return nullptr;
}

#else
EPEMPTYFILE
#endif
