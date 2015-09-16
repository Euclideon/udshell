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
    return extensions;
  }

protected:
  static const udFixedSlice<const udString> extensions;

  static udResult RegisterExtensions(Kernel *pKernel);

  ImageSource(const ComponentDesc *pType, Kernel *pKernel, udSharedString uid, udInitParams initParams)
    : DataSource(pType, pKernel, uid, initParams)
  {
    StreamRef ref = OpenStream(initParams["src"]);
    Create(ref);
  }

  void Create(StreamRef spSource);
};

} // namespace ud

#endif // _UD_IMAGESOURCE_H
