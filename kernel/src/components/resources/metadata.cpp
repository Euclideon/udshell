#include "metadata.h"

namespace kernel
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

  EPSHELL_APIVERSION, // epVersion
  EPSHELL_PLUGINVERSION, // pluginVersion

  "metadata", // id
  "Metadata", // displayName
  "Metadata resource", // description

  Slice<CPropertyDesc>(props, EPARRAYSIZE(props)), // properties
  Slice<CMethodDesc>(methods, EPARRAYSIZE(methods)) // methods
};

} // namespace kernel
