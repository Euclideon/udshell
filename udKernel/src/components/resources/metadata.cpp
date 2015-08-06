#include "metadata.h"

namespace ud
{

static PropertyDesc props[] =
{
  {
    {
      "numrecords", // id
      "NumRecords", // displayName
      "Number of records in the metadata", // description
      TypeDesc(PropertyType::Integer)
    },
    &Metadata::NumRecords,
    nullptr
  }
};
static MethodDesc methods[] =
{
  {
    {
      "insert",
      "Insert a record",
      TypeDesc(PropertyType::Void), // result
      {
        TypeDesc(PropertyType::Variant), // key
        TypeDesc(PropertyType::Variant) // value
      }
    },
    Method(&Metadata::InsertMethod),  // method
  },
  {
    {
      "remove",
      "Remove a record",
      TypeDesc(PropertyType::Void), // result
      {
        TypeDesc(PropertyType::Variant), // key
      }
    },
    Method(&Metadata::Remove),  // method
  },
  {
    {
      "exists",
      "Check if a record exists",
      TypeDesc(PropertyType::Boolean), // result
      {
        TypeDesc(PropertyType::Variant), // key
      }
    },
    Method(&Metadata::Exists) // method
  },
  {
    {
      "get",
      "Get a record",
      TypeDesc(PropertyType::Variant), // result
      {
        TypeDesc(PropertyType::Variant), // key
      }
    },
    Method(&Metadata::Get), // method
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
