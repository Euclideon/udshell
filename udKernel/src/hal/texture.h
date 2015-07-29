#pragma once
#ifndef UDTEXTURE_H
#define UDTEXTURE_H

enum udImageFormat
{
  udIF_RGBA8,
  udIF_BGRA8,

  udIF_R_F32,

  udIF_Max
};

enum udTextureType
{
  udTT_1D,
  udTT_1DArray,
  udTT_2D,
  udTT_2DArray,
  udTT_Cube,
  udTT_CubeArray,
  udTT_3D,
  udTT_Max
};

struct udTexture;

udTexture* udTexture_CreateTexture(udTextureType type, size_t width, size_t height, int levels, udImageFormat format);
void udTexture_DestroyTexture(udTexture **ppTex);

void udTexture_SetImageData(udTexture *pTex, int element, int level, void *pImage);

#endif // UDTEXTURE_H
