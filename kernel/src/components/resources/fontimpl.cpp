#include "components/resources/fontimpl.h"
#include "ep/cpp/component/resource/buffer.h"
#include "ep/cpp/component/resource/metadata.h"

#include "components/freetype.h"

namespace ep {

SHARED_CLASS(Font);

Array<const MethodInfo> Font::getMethods() const
{
  return {
    EP_MAKE_METHOD(rasterizeText, "Rasterise text to an image")
  };
}


FontImpl::FontImpl(Component *_pInstance, Variant::VarMap initParams)
  : ImplSuper(_pInstance)
{
  spFreeType = initParams["freetype"].as<FreeTypeRef>();
  spFontData = initParams["fontdata"].as<BufferRef>();
  fontIndex = initParams["fontindex"].as<size_t>();

  Slice<const void> buffer = spFontData->mapForRead();
  epscope(exit) { spFontData->unmap(); };

  pFont = spFreeType->createFont(buffer, fontIndex);

  Variant::VarMap::MapType md = spFreeType->getMetadata(pFont);
  MetadataRef spMD = pInstance->getMetadata();
  for (auto d : md)
    spMD->insert(std::move(d.key), std::move(d.value));
}

FontImpl::~FontImpl()
{
  spFreeType->destroyFont(pFont);
}

ArrayBufferRef FontImpl::rasterizeText(String text, int fontSize, Float3 fontColor, Float3 outlineColor) const
{
  return spFreeType->rasterizeText(pFont, text, fontSize, fontColor, outlineColor);
}

} //namespace ep
