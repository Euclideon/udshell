#include "hal/driver.h"

#if EPIMAGE_DRIVER == EPDRIVER_WIN32

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

epImage* epImage_ReadImage(void *pBuffer, size_t bufferLen, const char *)
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

  epImage *pOutput = (epImage*)udAlloc(sizeof(epImage) + frames*sizeof(epImageSurface));
  pOutput->pSurfaces = (epImageSurface*)&pOutput[1];
  pOutput->elements = frames;
  pOutput->mips = 1;

  pOutput->numMetadataEntries = 0;
  pOutput->pMetadata = nullptr;

  for (UINT i = 0; i < frames; ++i)
  {
    epImageSurface &surface = pOutput->pSurfaces[i];

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

void* epImage_WriteImage(epImage *pImage, const char *pFileExt, size_t *pOutputSize)
{
  pImage;
  pFileExt;

  *pOutputSize = 0;
  return nullptr;
}

#endif
