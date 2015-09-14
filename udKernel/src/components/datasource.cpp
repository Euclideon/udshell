#include "components/datasource.h"
#include "components/file.h"
#include "components/memstream.h"
#include "components/stream.h"
#include "kernel.h"

namespace ud
{

static CPropertyDesc props[] =
{
  {
    {
      "resourcecount", // id
      "Resource Count", // displayName
      "Number of resources the data source provides", // description
    },
    &DataSource::GetNumResources, // getter
    nullptr  // setter
  }
};
static CMethodDesc methods[] =
{
  {
    {
      "getresourcename", // id
      "Get a resource name by index", // description
    },
    &DataSource::GetResourceName  // method
  },
  {
    {
      "getresource", // id
      "Get a resource by name or index", // description
    },
    &DataSource::GetResourceByVariant  // method
  },
  {
    {
      "setresource", // id
      "Set a resource by name", // description
    },
    &DataSource::SetResource  // method
  },
  {
    {
      "countresource", // id
      "Count resources with a name prefix", // description
    },
    &DataSource::CountResources  // method
  }
};
ComponentDesc DataSource::descriptor =
{
  &Component::descriptor, // pSuperDesc

  UDSHELL_APIVERSION, // udVersion
  UDSHELL_PLUGINVERSION, // pluginVersion

  "datasource", // id
  "Data Source", // displayName
  "Provides data", // description

  udSlice<CPropertyDesc>(props, UDARRAYSIZE(props)), // propeties
  udSlice<CMethodDesc>(methods, UDARRAYSIZE(methods)) // propeties
};

StreamRef DataSource::OpenStream(const udVariant source)
{
  ComponentRef spComp = nullptr;
  StreamRef spSource = nullptr;

  //  const udVariant &flags = initParams["flags"];
  //  size_t f = flags.as<size_t>();

  if (source.is(udVariant::Type::String))
  {
    // path or url?
    spSource = pKernel->CreateComponent<File>({ { "path", source.asString() }, { "flags", FileOpenFlags::Read } });
    if (!spSource)
    {
      LogWarning(5, "\"src\" file path not found: {0}", source.asString());
      throw udR_File_OpenFailure;
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
      spSource = pKernel->CreateComponent<MemStream>({ { "buffer", spBuffer }, { "flags", FileOpenFlags::Read } });
    }
  }

  if (!spSource)
  {
    LogError("Unknown type for \"src\" init paramater");
    throw udR_InvalidParameter_;
  }

  return spSource;
}

DataSource::DataSource(const ComponentDesc *pType, Kernel *pKernel, udSharedString uid, udInitParams initParams)
  : Component(pType, pKernel, uid, initParams)
{

}

} // namespace ud
