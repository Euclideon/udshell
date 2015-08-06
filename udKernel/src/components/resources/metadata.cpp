#include "metadata.h"

namespace ud
{

static PropertyDesc props[] =
{
  {
    "numrecords", // id
    "NumRecords", // displayName
    "Number of records in the metadata", // description
    &Metadata::NumRecords,
    nullptr,
    TypeDesc(PropertyType::Integer)
  }
};
static MethodDesc methods[] =
{
  {
    "insert",
    "Insert a record",
    Method(&Metadata::InsertMethod),
    TypeDesc(PropertyType::Void), // result
    {
      TypeDesc(PropertyType::Variant), // key
      TypeDesc(PropertyType::Variant) // value
    }
  },
  {
    "remove",
    "Remove a record",
    Method(&Metadata::Remove),
    TypeDesc(PropertyType::Void), // result
    {
      TypeDesc(PropertyType::Variant), // key
    }
  },
  {
    "exists",
    "Check if a record exists",
    Method(&Metadata::Exists),
    TypeDesc(PropertyType::Boolean), // result
    {
      TypeDesc(PropertyType::Variant), // key
    }
  },
  {
    "get",
    "Get a record",
    Method(&Metadata::Get),
    TypeDesc(PropertyType::Variant), // result
    {
      TypeDesc(PropertyType::Variant), // key
    }
  }
};
ComponentDesc Metadata::descriptor =
{
  &Resource::descriptor, // pSuperDesc

  UDSHELL_APIVERSION, // udVersion
  UDSHELL_PLUGINVERSION, // pluginVersion

  "metadata", // id
  "Metadata", // displayName
  "Metadata resource", // description

  udSlice<PropertyDesc>(props, UDARRAYSIZE(props)), // properties
  udSlice<MethodDesc>(methods, UDARRAYSIZE(methods)) // methods
};

} // namespace ud
