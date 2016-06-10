#include "components/datasources/datasourceimpl.h"
#include "ep/cpp/component/file.h"
#include "components/memstream.h"
#include "ep/cpp/component/stream.h"
#include "ep/cpp/kernel.h"

namespace ep {

Array<const PropertyInfo> DataSource::GetProperties() const
{
  return{
    EP_MAKE_PROPERTY_RO(NumResources, "Number of resources the data source provides", nullptr, 0),
    EP_MAKE_PROPERTY(URL, "The file or location of the DataSource", nullptr, 0),
    EP_MAKE_PROPERTY_RO(FileExtensions, "The file extensions associated with the DataSource", nullptr, 0),
  };
}
Array<const MethodInfo> DataSource::GetMethods() const
{
  return{
    EP_MAKE_METHOD(GetResourceName, "Get a resource name by index"),
    EP_MAKE_METHOD(GetResourceByVariant, "Get a resource by name or index"),
    EP_MAKE_METHOD(SetResource, "Set a resource by name"),
    EP_MAKE_METHOD(CountResources, "Count resources with a name prefix"),
  };
}


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
    EPTHROW_ERROR(Result::InvalidArgument, "Unknown type for \"src\" init paramater");

  return spSource;
}

DataSourceImpl::DataSourceImpl(Component *pInstance, Variant::VarMap initParams)
  : ImplSuper(pInstance)
{
  const Variant *source = initParams.get("src");
  if (source && source->is(Variant::Type::String))
    url = source->asString();
}

} // namespace ep
