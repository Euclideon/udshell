#include "components/datasources/fontsource.h"
#include "components/freetype.h"
#include "ep/cpp/component/file.h"
#include "ep/cpp/component/resource/metadata.h"

namespace ep {

const Array<const String> FontSource::extensions = { ".ttf", ".ttc", ".otf", ".woff", ".pfa", ".pfb", ".fon", ".fnt", ".pcf", ".bdf", ".pfr" };

FreeTypeRef FontSource::spFreeType;

FontSource::FontSource(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
  : DataSource(pType, pKernel, uid, initParams)
{
  if (!spFreeType)
    spFreeType = pKernel->createComponent<FreeType>();

  // load the font
  StreamRef spStream = openStream(*initParams.get("src"));
  BufferRef spBuffer = spStream->load();

  // map the font memory
  FontRef spFont = pKernel->createComponent<Font>({ { "freetype", spFreeType }, { "fontdata", spBuffer }, { "fontindex", 0 } });
  setResource(MutableString64(Concat, "font", 0), spFont);

  // are there more faces?
  size_t numFaces = spFont->getMetadata()->get("num_faces").as<size_t>();
  for (size_t i = 1; i < numFaces; ++i)
  {
    spFont = pKernel->createComponent<Font>({ { "freetype", spFreeType }, { "fontdata", spBuffer }, { "fontindex", i } });
    setResource(MutableString64(Concat, "font", i), spFont);
  }
}

void FontSource::staticInit(ep::Kernel *pKernel)
{
   pKernel->registerExtensions(pKernel->getComponentDesc(componentID()), extensions);
}

} // namespace ep
