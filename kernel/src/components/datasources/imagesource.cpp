#include "imagesource.h"
#include "ep/cpp/component/resource/arraybuffer.h"
#include "ep/cpp/kernel.h"
#include "hal/image.h"

namespace ep {

const Array<const String> ImageSource::extensions = { ".bmp", ".png", ".jpg", ".jpeg", ".gif", ".tiff", ".tif", ".tga", ".dds", ".webp" };

const char* GetFormatString(epImageFormat format)
{
  if (format == epIF_RGBA8) return "u8[4]";
  if (format == epIF_BGRA8) return "u32";

  // TODO: add support for more ep formats
  EPTHROW(Result::Failure, "Image format not supported yet");
}

void ImageSource::Create(StreamRef spSource)
{
  // allocate for file
  int64_t len = spSource->Length();
  void *pBuffer = epAlloc((size_t)len);
  EPTHROW_IF_NULL(pBuffer, Result::AllocFailure, "Memory allocation failed");
  epscope(exit) { epFree(pBuffer); };

  // read file from source
  Slice<void> buf(pBuffer, (size_t)len);
  buf = spSource->Read(buf);
  EPASSERT((int64_t)buf.length == len, "!");

  // load the image
  epImage *pImage = epImage_LoadImage(pBuffer, (size_t)len, nullptr);
  EPTHROW_IF_NULL(pImage, Result::AllocFailure, "Memory allocation failed");
  epscope(exit) { epImage_DestroyImage(&pImage); };

  for (size_t i = 0; i<pImage->elements; ++i)
  {
    epImageSurface &s = pImage->pSurfaces[i];

    // create image for each image element
    ArrayBufferRef spImage = getKernel().createComponent<ArrayBuffer>();
    spImage->allocate(GetFormatString(s.format), 4, Slice<const size_t>{ s.width, s.height });

    // write image to to the array buffer
    Slice<void> mem = spImage->map();
    EPASSERT(mem.length == s.width*s.height*4, "Wrong size?!");
//    mem.copyTo(Slice<void>(s.pImage, s.width*s.height*4)); // TODO: use copyTo()...
    memcpy(mem.ptr, s.pImage, s.width*s.height*4);
    spImage->unmap();

    // add resource
    setResource(MutableString64(Concat, "image", i), spImage);
  }
}

void ImageSource::StaticInit(ep::Kernel *pKernel)
{
   pKernel->registerExtensions(pKernel->getComponentDesc(componentID()), extensions);
}

} // namespace ep
