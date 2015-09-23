#include "imagesource.h"
#include "components/resources/array.h"
#include "hal/image.h"
#include "kernel.h"

namespace ud
{
const udFixedSlice<const udString> ImageSource::extensions = { ".bmp", ".png", ".jpg", ".jpeg", ".gif", ".tiff", ".tif", ".tga", ".dds", ".webp" };

ComponentDesc ImageSource::descriptor =
{
  &DataSource::descriptor, // pSuperDesc

  UDSHELL_APIVERSION, // udVersion
  UDSHELL_PLUGINVERSION, // pluginVersion

  "imagesource", // id
  "Image Source", // displayName
  "Provides images", // description

  nullptr,            // properties
  nullptr,            // methods
  nullptr,            // events
  nullptr,            // static functions
  &RegisterExtensions // init
};

void ImageSource::Create(StreamRef spSource)
{
  // allocate for file
  int64_t len = spSource->Length();
  void *pBuffer = udAlloc((size_t)len);

  // read file from source
  udSlice<void> buf(pBuffer, (size_t)len);
  buf = spSource->Read(buf);
  UDASSERT((int64_t)buf.length == len, "!");

  // load the image
  udImage *pImage = udImage_ReadImage(pBuffer, (size_t)len, nullptr);

  for (size_t i = 0; i<pImage->elements; ++i)
  {
    udImageSurface &s = pImage->pSurfaces[i];

    // create image for each image element
    ArrayBufferRef spImage = pKernel->CreateComponent<ArrayBuffer>();
    spImage->Allocate("{u8[4]}", 4, { s.width, s.height });

    // write image to to the array buffer
    udSlice<void> mem = spImage->Map();
    UDASSERT(mem.length == s.width*s.height*4, "Wrong size?!");
//    mem.copyTo(udSlice<void>(s.pImage, s.width*s.height*4)); // TODO: use copyTo()...
    memcpy(mem.ptr, s.pImage, s.width*s.height*4);
    spImage->Unmap();

    // add resource
    udMutableString64 buffer; buffer.concat("image", i);
    resources.Insert(buffer, spImage);
  }

  udFree(pBuffer);
}

udResult ImageSource::RegisterExtensions(Kernel *pKernel)
{
  return pKernel->RegisterExtensions(&descriptor, extensions);
}

} // namespace ud
