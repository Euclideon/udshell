#pragma once
#ifndef EPTEXTURE_H
#define EPTEXTURE_H

enum epImageFormat
{
  epIF_RGBA8,
  epIF_BGRA8,

  epIF_R_F32,

  epIF_Max
};

enum epTextureType
{
  epTT_1D,
  epTT_1DArray,
  epTT_2D,
  epTT_2DArray,
  epTT_Cube,
  epTT_CubeArray,
  epTT_3D,
  epTT_Max
};

struct epTexture;

epTexture* epTexture_CreateTexture(epTextureType type, size_t width, size_t height, int levels, epImageFormat format);
void epTexture_DestroyTexture(epTexture **ppTex);

void epTexture_SetImageData(epTexture *pTex, int element, int level, void *pImage);

#endif // EPTEXTURE_H
