#pragma once
#if !defined(_UD_IMAGESOURCE_H)
#define _UD_IMAGESOURCE_H

#include "components/datasource.h"
#include "components/resources/array.h"
#include "components/stream.h"

namespace ud
{

SHARED_CLASS(ImageSource);

class ImageSource : public DataSource
{
public:
  UD_COMPONENT(ImageSource);

  udSlice<const udString> GetFileExtensions() const override
  {
    return{ ".bmp", ".png", ".jpg", ".jpeg", ".gif", ".tiff", ".tif", ".tga", ".dds", ".webp" };
  }

protected:
  ImageSource(const ComponentDesc *pType, Kernel *pKernel, udSharedString uid, udInitParams initParams)
    : DataSource(pType, pKernel, uid, initParams)
  {}

  void Create(StreamRef spSource) override;
};

} // namespace ud

#endif // _UD_IMAGESOURCE_H
