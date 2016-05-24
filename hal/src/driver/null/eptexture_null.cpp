#include "driver.h"

#if EPRENDER_DRIVER == EPDRIVER_NULL

#include "hal/texture.h"

// ***************************************************************************************
// Author: Manu Evans, May 2015
epTexture* epTexture_CreateTexture(epTextureType type, size_t width, size_t height, int levels, epImageFormat format)
{
  return nullptr;
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
void epTexture_DestroyTexture(epTexture **ppTex)
{
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
void epTexture_SetImageData(epTexture *pTex, int element, int level, const void *pImage)
{
}

#else
EPEMPTYFILE
#endif // EPRENDER_DRIVER == EPDRIVER_OPENGL
