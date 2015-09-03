#include "components/datasource.h"

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

DataSource::DataSource(const ComponentDesc *pType, Kernel *pKernel, udSharedString uid, udInitParams initParams)
  : Component(pType, pKernel, uid, initParams)
{
  const udVariant &source = initParams["source"];

//  const udVariant &flags = initParams["flags"];
//  size_t f = flags.as<size_t>();

  StreamRef spSource;
  if (source.is(udVariant::Type::String))
  {
    // path or url?
//    pKernel->CreateComponent<File>({ "path", source.toString() });

    // open a file stream...
  }
  else if (source.is(udVariant::Type::Component))
  {
    // already a stream...

    // binary buffer? (formatted image)   (source is read-only)
    //   wrap stream around buffer
  }

  Create(spSource);
}

} // namespace ud
