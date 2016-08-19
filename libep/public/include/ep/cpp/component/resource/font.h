#pragma once
#ifndef _EP_FONT_HPP
#define _EP_FONT_HPP

#include "ep/cpp/component/resource/resource.h"
#include "ep/cpp/internal/i/ifont.h"

namespace ep {

SHARED_CLASS(Font);

class Font : public Resource
{
  EP_DECLARE_COMPONENT_WITH_IMPL(ep, Font, IFont, Resource, EPKERNEL_PLUGINVERSION, "Font resource for text rendering", 0);

public:
  ArrayBufferRef rasterizeText(String text, int fontSize, Float3 fontColor, Float3 outlineColor) { return pImpl->rasterizeText(text, fontSize, fontColor, outlineColor); }

  Variant save() const override { return pImpl->save(); }

protected:
  Font(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
    : Resource(pType, pKernel, uid, initParams)
  {
    pImpl = createImpl(initParams);
  }

private:
  Array<const MethodInfo> getMethods() const;
};

} //namespace ep

#endif // _EP_FONT_HPP
