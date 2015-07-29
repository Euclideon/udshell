#include "udDriver.h"

#if UDRENDER_DRIVER == UDDRIVER_NULL

#include "udTexture.h"

// ***************************************************************************************
// Author: Manu Evans, May 2015
udTexture* udTexture_CreateTexture(udTextureType type, size_t width, size_t height, int levels, udImageFormat format)
{
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
void udTexture_DestroyTexture(udTexture **ppTex)
{
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
void udTexture_SetImageData(udTexture *pTex, int element, int level, void *pImage)
{
}

#endif // UDRENDER_DRIVER == UDDRIVER_OPENGL
