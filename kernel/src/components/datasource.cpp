#include "components/datasource.h"
#include "components/file.h"
#include "components/memstream.h"
#include "components/stream.h"
#include "kernel.h"

namespace kernel
{

StreamRef DataSource::OpenStream(const Variant &source)
{
  ComponentRef spComp = nullptr;
  StreamRef spSource = nullptr;

  //  const Variant &flags = initParams["flags"];
  //  size_t f = flags.as<size_t>();

  if (source.is(Variant::Type::String))
  {
    // path or url?
    spSource = GetKernel().CreateComponent<File>({ { "path", source }, { "flags", FileOpenFlags::Read } });
    if (!spSource)
    {
      LogWarning(5, "\"src\" file path not found: {0}", source.asString());
      throw epR_File_OpenFailure;
    }
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
      spSource = GetKernel().CreateComponent<MemStream>({ { "buffer", spBuffer }, { "flags", FileOpenFlags::Read } });
    }
  }

  if (!spSource)
  {
    LogError("Unknown type for \"src\" init paramater");
    throw epR_InvalidParameter;
  }

  return spSource;
}

DataSource::DataSource(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
  : Component(pType, pKernel, uid, initParams)
{
  const Variant &source = initParams["src"];
  if (source.is(Variant::Type::String))
    url = source.asString();
}

} // namespace kernel
