#pragma once
#if !defined(_EP_FONTSOURCE_H)
#define _EP_FONTSOURCE_H

#include "ep/cpp/component/datasource/datasource.h"
#include "ep/cpp/component/resource/font.h"

namespace ep {

SHARED_CLASS(FontSource);
SHARED_CLASS(FreeType);

class FontSource : public DataSource
{
  EP_DECLARE_COMPONENT(ep, FontSource, DataSource, EPKERNEL_PLUGINVERSION, "Provides fonts", 0)
public:

  Slice<const String> getFileExtensions() const override { return extensions; }
  static Slice<const String> staticGetFileExtensions() { return extensions; }

protected:
  FontSource(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams);

  static const Array<const String> extensions;
  static FreeTypeRef spFreeType;

  static void staticInit(ep::Kernel *pKernel);
};

} // namespace ep

#endif // _EP_FONTSOURCE_H
