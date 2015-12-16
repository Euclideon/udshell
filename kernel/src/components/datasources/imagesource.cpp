#include "imagesource.h"
#include "components/resources/array.h"
#include "hal/image.h"
#include "kernel.h"

namespace ep {

const Array<const String> ImageSource::extensions = { ".bmp", ".png", ".jpg", ".jpeg", ".gif", ".tiff", ".tif", ".tga", ".dds", ".webp" };

void ImageSource::Create(StreamRef spSource)
{
  // allocate for file
  int64_t len = spSource->Length();
  void *pBuffer = epAlloc((size_t)len);

  // read file from source
  Slice<void> buf(pBuffer, (size_t)len);
  buf = spSource->Read(buf);
  EPASSERT((int64_t)buf.length == len, "!");

  // load the image
  epImage *pImage = epImage_ReadImage(pBuffer, (size_t)len, nullptr);

  for (size_t i = 0; i<pImage->elements; ++i)
  {
    epImageSurface &s = pImage->pSurfaces[i];

    // create image for each image element
    ArrayBufferRef spImage = GetKernel().CreateComponent<ArrayBuffer>();
    spImage->Allocate("{u8[4]}", 4, { s.width, s.height });

    // write image to to the array buffer
    Slice<void> mem = spImage->Map();
    EPASSERT(mem.length == s.width*s.height*4, "Wrong size?!");
//    mem.copyTo(Slice<void>(s.pImage, s.width*s.height*4)); // TODO: use copyTo()...
    memcpy(mem.ptr, s.pImage, s.width*s.height*4);
    spImage->Unmap();

    // add resource
    resources.Insert(MutableString64(Concat, "image", i), spImage);
  }

  epFree(pBuffer);
}

epResult ImageSource::StaticInit(ep::Kernel *pKernel)
{
  return pKernel->RegisterExtensions(pKernel->GetComponentDesc(ComponentID()), extensions);
}

} // namespace ep
