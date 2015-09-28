#include "kvpstore.h"

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
    &KVPStore::NumRecords,
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
    &KVPStore::InsertMethod,  // method
  },
  {
    {
      "remove",
      "Remove a record",
    },
    &KVPStore::Remove,  // method
  },
  {
    {
      "exists",
      "Check if a record exists",
    },
    &KVPStore::Exists // method
  },
  {
    {
      "get",
      "Get a record",
    },
    &KVPStore::Get, // method
  }
};
ComponentDesc KVPStore::descriptor =
{
  &Resource::descriptor, // pSuperDesc

  EPSHELL_APIVERSION, // epVersion
  EPSHELL_PLUGINVERSION, // pluginVersion

  "metadata", // id
  "Metadata", // displayName
  "Metadata resource", // description

  epSlice<CPropertyDesc>(props, UDARRAYSIZE(props)), // properties
  epSlice<CMethodDesc>(methods, UDARRAYSIZE(methods)) // methods
};

} // namespace ep
