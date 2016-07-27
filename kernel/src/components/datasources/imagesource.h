#pragma once
#if !defined(_EP_IMAGESOURCE_H)
#define _EP_IMAGESOURCE_H

#include "ep/cpp/component/datasource/datasource.h"
#include "ep/cpp/component/resource/arraybuffer.h"
#include "ep/cpp/component/stream.h"

namespace ep
{

SHARED_CLASS(ImageSource);

class ImageSource : public DataSource
{
  EP_DECLARE_COMPONENT(ep, ImageSource, DataSource, EPKERNEL_PLUGINVERSION, "Provides images", 0)
public:

  Slice<const String> GetFileExtensions() const override
  {
    return extensions;
  }

  static Slice<const String> StaticGetFileExtensions() { return extensions; }

protected:
  static const Array<const String> extensions;

  static void StaticInit(ep::Kernel *pKernel);

  ImageSource(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
    : DataSource(pType, pKernel, uid, initParams)
  {
    StreamRef ref = openStream(*initParams.get("src"));
    Create(ref);
  }

  void Create(StreamRef spSource);
};

} // namespace ep

#endif // _EP_IMAGESOURCE_H
