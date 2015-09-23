#include "hal/driver.h"

#if UDIMAGE_DRIVER == UDDRIVER_WIN32

#include "hal/image.h"

#include <wincodec.h>
#include <wincodecsdk.h>

static IWICImagingFactory *pFactory = nullptr;

void udImage_InitInternal()
{
  CoInitialize(nullptr);

  HRESULT hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr,
                                CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pFactory));
  if (FAILED(hr))
    udDebugPrintf("Failed to create WICImagingFactory");
}

void udImage_DeinitInternal()
{
  if (pFactory)
    pFactory->Release();
}

static udImageFormat GUIDToImageFormat(WICPixelFormatGUID format)
{
  if (format == GUID_WICPixelFormat32bppBGRA)
    return udIF_BGRA8;
  else  if (format == GUID_WICPixelFormat32bppRGBA)
    return udIF_RGBA8;
  else  if (format == GUID_WICPixelFormat32bppBGR)
    return udIF_BGR8;
  else  if (format == GUID_WICPixelFormat32bppRGB)
    return udIF_RGB8;
  return udIF_Unknown;
}

udImage* udImage_ReadImage(void *pBuffer, size_t bufferLen, const char *)
{
  HGLOBAL hGlobal = GlobalAlloc(0, bufferLen);
  void *pMem = GlobalLock(hGlobal);
  memcpy(pMem, pBuffer, bufferLen);
  GlobalUnlock(hGlobal);

  IStream *pStream = nullptr;
  CreateStreamOnHGlobal(hGlobal, true, &pStream);

  IWICBitmapDecoder *pDecoder;
  pFactory->CreateDecoderFromStream(pStream, nullptr, WICDecodeMetadataCacheOnLoad, &pDecoder);

  UINT frames;
  pDecoder->GetFrameCount(&frames);

  udImage *pOutput = (udImage*)udAlloc(sizeof(udImage) + frames*sizeof(udImageSurface));
  pOutput->pSurfaces = (udImageSurface*)&pOutput[1];
  pOutput->elements = frames;
  pOutput->mips = 1;

  pOutput->numMetadataEntries = 0;
  pOutput->pMetadata = nullptr;

  for (UINT i = 0; i < frames; ++i)
  {
    udImageSurface &surface = pOutput->pSurfaces[i];

    IWICBitmapFrameDecode *pBitmapSource;
    pDecoder->GetFrame(0, &pBitmapSource);

    // TODO: ** don't create converter! we should load the image in it's raw format
    // HACK: we convert to BGRA8 for now
    IWICFormatConverter *pConverter;
    pFactory->CreateFormatConverter(&pConverter);
    pConverter->Initialize(pBitmapSource, GUID_WICPixelFormat32bppBGRA,
                           WICBitmapDitherTypeNone, nullptr, 0.f,
                           WICBitmapPaletteTypeMedianCut);

    UINT w, h;
    pBitmapSource->GetSize(&w, &h);
    surface.width = w;
    surface.height = h;
    surface.depth = 0;

    WICPixelFormatGUID format;
    pBitmapSource->GetPixelFormat(&format);
    surface.format = GUIDToImageFormat(format);

    surface.pImage = udAlloc(w*h*4);
    pBitmapSource->CopyPixels(nullptr, w*4, w*h*4, (BYTE*)surface.pImage);

    // this is useful metadata
//    double wdpi, hdpi;
//    pBitmapSource->GetResolution(&wdpi, &hdpi);

    pConverter->Release();
    pBitmapSource->Release();
  }

  pDecoder->Release();
  pStream->Release();

  return pOutput;
}

void* udImage_WriteImage(udImage *pImage, const char *pFileExt, size_t *pOutputSize)
{
  pImage;
  pFileExt;

  *pOutputSize = 0;
  return nullptr;
}

#endif
