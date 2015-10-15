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
public:
  EP_COMPONENT(ImageSource);

  Slice<const String> GetFileExtensions() const override
  {
    return extensions;
  }

protected:
  static const Array<const String> extensions;

  static epResult RegisterExtensions(Kernel *pKernel);

  ImageSource(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, InitParams initParams)
    : DataSource(pType, pKernel, uid, initParams)
  {
    StreamRef ref = OpenStream(initParams["src"]);
    Create(ref);
  }

  void Create(StreamRef spSource);
};

} // namespace ep

#endif // _EP_IMAGESOURCE_H
