#include "udDriver.h"

#if UDRENDER_DRIVER == UDDRIVER_QT

#include "udTexture.h"


// ***************************************************************************************
udTexture* udTexture_CreateTexture(udTextureType type, size_t width, size_t height, int levels, udImageFormat format)
{
  return 0;
}

// ***************************************************************************************
void udTexture_DestroyTexture(udTexture **ppTex)
{
}

// ***************************************************************************************
void udTexture_SetImageData(udTexture *pTex, int element, int level, void *pImage)
{
}

#endif // UDRENDER_DRIVER == UDDRIVER_QT
