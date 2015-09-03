#include "imagesource.h"
#include "components/resources/array.h"
#include "hal/image.h"
#include "kernel.h"

namespace ud
{

ComponentDesc ImageSource::descriptor =
{
  &DataSource::descriptor, // pSuperDesc

  UDSHELL_APIVERSION, // udVersion
  UDSHELL_PLUGINVERSION, // pluginVersion

  "imagesource", // id
  "Image Source", // displayName
  "Provides images", // description
};

void ImageSource::Create(StreamRef spSource)
{
  // allocate for file
  int64_t len = spSource->Length();
  void *pBuffer = udAlloc(len);

  // read file from source
  size_t read = spSource->Read(pBuffer, len);
  UDASSERT((int64_t)read == len, "!");

  // load the image
  udImage *pImage = udImage_ReadImage(pBuffer, len, nullptr);

  for (size_t i = 0; i<pImage->elements; ++i)
  {
    udImageSurface &s = pImage->pSurfaces[i];

    // create image for each image element
    ArrayBufferRef spImage = pKernel->CreateComponent<ArrayBuffer>();
    spImage->Allocate("{u8[4]}", 4, { s.width, s.height });

    // write image to to the array buffer
    size_t size;
    void *pMem = spImage->Map(&size);
    UDASSERT(size == s.width*s.height*4, "Wrong size?!");
    memcpy(pMem, s.pImage, s.width*s.height*4);
    spImage->Unmap();

    // add resource
    udMutableString64 buffer; buffer.concat("image", i);
    resources.Insert(buffer, spImage);
  }
}

} // namespace ud
