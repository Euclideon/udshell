#include "components/datasource.h"

namespace ud
{

static PropertyDesc props[] =
{
  {
    "resourcecount", // id
    "Resource Count", // displayName
    "Number of resources the data source provides", // description
    &DataSource::GetNumResources, // getter
    nullptr, // setter
    TypeDesc(PropertyType::Integer) // type
  },
  {
    "resources", // id
    "Resources", // displayName
    "List of resources", // description
    &DataSource::GetResources, // getter
    nullptr, // setter
    TypeDesc(PropertyType::Resource, ~0), // type
  }
};
static MethodDesc methods[] =
{
  {
    "getresource", // id
    "Get Resource", // displayName
    "Get a resource by index", // description
    &DataSource::GetResource, // method
    TypeDesc(PropertyType::Resource), // type
    {
      TypeDesc(PropertyType::Integer), // arg0
    }
  },
  {
    "getresourcetype", // id
    "Get Resource Type", // displayName
    "Get the type of a resource", // description
    &DataSource::GetResourceType, // method
    TypeDesc(PropertyType::Integer), // type
    {
      TypeDesc(PropertyType::Integer), // arg0
    }
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

  udSlice<PropertyDesc>(props, UDARRAYSIZE(props)), // propeties
  udSlice<MethodDesc>(methods, UDARRAYSIZE(methods)) // propeties
};

DataSource::DataSource(const ComponentDesc *pType, Kernel *pKernel, udRCString uid, udInitParams initParams)
  : Component(pType, pKernel, uid, initParams)
{
  ComponentRef spSource, spData;

  const udVariant &source = initParams["source"];
  const udVariant &data = initParams["data"];

  if (data.is(udVariant::Type::Component))
  {
    // binary buffer? (pre-formatted image)
    // array buffer? (raw image)
  }

  if (source.is(udVariant::Type::String))
  {
    // path or url?

    // open a stream...
  }
  else if (source.is(udVariant::Type::Component))
  {
    // binary buffer? (formatted image)   (source is read-only)
  }

  Create(spSource, spData);
}

} // namespace ud
