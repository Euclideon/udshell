#include "metadata.h"

namespace ep
{

static CPropertyDesc props[] =
{
  {
    {
      "numrecords", // id
      "NumRecords", // displayName
      "Number of records in the metadata", // description
    },
    &Metadata::NumRecords,
    nullptr
  }
};
static CMethodDesc methods[] =
{
  {
    {
      "insert",
      "Insert a record",
    },
    &Metadata::InsertMethod,  // method
  },
  {
    {
      "remove",
      "Remove a record",
    },
    &Metadata::Remove,  // method
  },
  {
    {
      "exists",
      "Check if a record exists",
    },
    &Metadata::Exists // method
  },
  {
    {
      "get",
      "Get a record",
    },
    &Metadata::Get, // method
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

  epSlice<CPropertyDesc>(props, UDARRAYSIZE(props)), // properties
  epSlice<CMethodDesc>(methods, UDARRAYSIZE(methods)) // methods
};

} // namespace ep
