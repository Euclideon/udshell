#include "driver.h"

#if EPIMAGE_DRIVER == EPDRIVER_WIN32

#include "hal/image.h"

#include <wincodec.h>
#include <wincodecsdk.h>

static IWICImagingFactory *pFactory = nullptr;

void epImage_InitInternal()
{
  CoInitialize(nullptr);

  HRESULT hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr,
                                CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pFactory));
  if (FAILED(hr))
    epDebugPrintf("Failed to create WICImagingFactory\n");
}

void epImage_DeinitInternal()
{
  if (pFactory)
    pFactory->Release();
}

static epImageFormat GUIDToImageFormat(WICPixelFormatGUID format)
{
  if (format == GUID_WICPixelFormat32bppBGRA)
    return epIF_BGRA8;
  else  if (format == GUID_WICPixelFormat32bppRGBA)
    return epIF_RGBA8;
  else  if (format == GUID_WICPixelFormat32bppBGR)
    return epIF_BGR8;
  else  if (format == GUID_WICPixelFormat32bppRGB)
    return epIF_RGB8;
  return epIF_Unknown;
}

epImage* epImage_LoadImage(void *pBuffer, size_t bufferLen, const char *)
{
  epImage *pOutput = nullptr;

  HGLOBAL hGlobal = GlobalAlloc(0, bufferLen);
  if (!hGlobal)
  {
    epDebugPrintf("Error allocating image buffer\n");
    return nullptr;
  }
  void *pMem = GlobalLock(hGlobal);
  memcpy(pMem, pBuffer, bufferLen);
  GlobalUnlock(hGlobal);

  IStream *pStream = nullptr;
  if (S_OK != CreateStreamOnHGlobal(hGlobal, true, &pStream))
  {
    GlobalFree(hGlobal);
    epDebugPrintf("Error creating stream for image buffer\n");
    return nullptr;
  }

  IWICBitmapDecoder *pDecoder;
  if (S_OK != pFactory->CreateDecoderFromStream(pStream, nullptr, WICDecodeMetadataCacheOnLoad, &pDecoder))
  {
    pStream->Release();
    epDebugPrintf("Error creating decoder from image data\n");
    return nullptr;
  }

  UINT frames;
  if (S_OK != pDecoder->GetFrameCount(&frames))
  {
    epDebugPrintf("Error getting image frame count\n");
    goto epilogue;
  }

  pOutput = (epImage*)epAllocFlags(sizeof(epImage) + frames*sizeof(epImageSurface), epAF_Zero);
  if (!pOutput)
  {
    epDebugPrintf("Error allocating epImage\n");
    goto epilogue;
  }

  pOutput->pSurfaces = (epImageSurface*)&pOutput[1];
  pOutput->elements = frames;
  pOutput->mips = 1;

  pOutput->numMetadataEntries = 0;
  pOutput->pMetadata = nullptr;

  for (UINT i = 0; i < frames; ++i)
  {
    epImageSurface &surface = pOutput->pSurfaces[i];

    IWICBitmapFrameDecode *pBitmapSource;
    if (S_OK != pDecoder->GetFrame(0, &pBitmapSource))
    {
      epDebugPrintf("Error getting image frame\n");
      goto freeImage;
    }

    // TODO: ** don't create converter! we should load the image in it's raw format
    // HACK: we convert to BGRA8 for now
    IWICFormatConverter *pConverter;
    pFactory->CreateFormatConverter(&pConverter);
    if(S_OK != pConverter->Initialize(pBitmapSource, GUID_WICPixelFormat32bppBGRA,
      WICBitmapDitherTypeNone, nullptr, 0.f,
      WICBitmapPaletteTypeMedianCut))
    {
      epDebugPrintf("Error converting image to BGRA32 format\n");
      pConverter->Release();
      pBitmapSource->Release();
      goto freeImage;
    }

    UINT w, h;
    pBitmapSource->GetSize(&w, &h);
    surface.width = w;
    surface.height = h;
    surface.depth = 0;

    WICPixelFormatGUID format;
    pBitmapSource->GetPixelFormat(&format);
    surface.format = GUIDToImageFormat(format);

    surface.pImage = epAlloc(w*h*4);
    if (!surface.pImage)
    {
      epDebugPrintf("Error allocating epImage frame\n");
      pConverter->Release();
      pBitmapSource->Release();
      goto freeImage;
    }
    pBitmapSource->CopyPixels(nullptr, w*4, w*h*4, (BYTE*)surface.pImage);

    // this is useful metadata
//    double wdpi, hdpi;
//    pBitmapSource->GetResolution(&wdpi, &hdpi);

    pConverter->Release();
    pBitmapSource->Release();
  }

  goto epilogue;

freeImage:
  if (pOutput)
  {
    for (UINT i = 0; i < frames; i++)
      epFree(pOutput->pSurfaces[i].pImage);
    epFree(pOutput);
  }
  pOutput = nullptr;

epilogue:
  pDecoder->Release();
  pStream->Release();

  return pOutput;
}

void epImage_DestroyImage(epImage **ppImage)
{
  if (ppImage && *ppImage)
  {
    for (size_t i = 0; i < (*ppImage)->elements; ++i)
      epFree((*ppImage)->pSurfaces[i].pImage);
    epFree(*ppImage);
    *ppImage = nullptr;
  }
}

void* epImage_WriteImage(epImage epUnusedParam(*pImage), const char epUnusedParam(*pFileExt), size_t epUnusedParam(*pOutputSize))
{
  return nullptr;
}

#else
EPEMPTYFILE
#endif
