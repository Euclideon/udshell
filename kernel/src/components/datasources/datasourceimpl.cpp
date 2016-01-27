#include "components/datasources/datasourceimpl.h"
#include "components/file.h"
#include "components/memstream.h"
#include "ep/cpp/component/stream.h"
#include "kernel.h"

namespace ep
{

StreamRef DataSourceImpl::OpenStream(const Variant &source)
{
  ComponentRef spComp = nullptr;
  StreamRef spSource = nullptr;

  //  const Variant &flags = initParams["flags"];
  //  size_t f = flags.as<size_t>();

  if (source.is(Variant::Type::String))
  {
    // path or url?
    spSource = GetKernel()->CreateComponent<File>({ { "path", source }, { "flags", FileOpenFlags::Read } });
    if (!spSource)
      EPTHROW_WARN(epR_File_OpenFailure, 2, "\"src\" file path not found: {0}", source.asString());
  }
  else if ((spComp = source.as<ComponentRef>()))
  {
    if (spComp->IsType<Stream>())
    {
      spSource = shared_pointer_cast<Stream>(spComp);
    }
    else if (spComp->IsType<Buffer>())
    {
      BufferRef spBuffer = shared_pointer_cast<Buffer>(spComp);
      spSource = GetKernel()->CreateComponent<MemStream>({ { "buffer", spBuffer }, { "flags", FileOpenFlags::Read } });
    }
  }

  if (!spSource)
    EPTHROW_ERROR(epR_InvalidArgument, "Unknown type for \"src\" init paramater");

  return spSource;
}

DataSourceImpl::DataSourceImpl(Component *pInstance, Variant::VarMap initParams)
  : ImplSuper(pInstance)
{
  const Variant *source = initParams.Get("src");
  if (source && source->is(Variant::Type::String))
    url = source->asString();
}

} // namespace ep
