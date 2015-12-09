#pragma once
#if !defined(_EP_IMAGESOURCE_H)
#define _EP_IMAGESOURCE_H

#include "components/datasource.h"
#include "components/resources/array.h"
#include "components/stream.h"

namespace ep
{

SHARED_CLASS(ImageSource);

class ImageSource : public DataSource
{
  EP_DECLARE_COMPONENT(ImageSource, DataSource, EPKERNEL_PLUGINVERSION, "Provides images")
public:

  Slice<const String> GetFileExtensions() const override
  {
    return extensions;
  }

protected:
  static const Array<const String> extensions;

  static epResult StaticInit(ep::Kernel *pKernel);

  ImageSource(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
    : DataSource(pType, pKernel, uid, initParams)
  {
    StreamRef ref = OpenStream(*initParams.Get("src"));
    Create(ref);
  }

  void Create(StreamRef spSource);
};

} // namespace ep

#endif // _EP_IMAGESOURCE_H
