#include "ep/cpp/component/resource/kvpstore.h"

namespace ep {

Array<const PropertyInfo> KVPStore::getProperties() const
{
  return{
    EP_MAKE_PROPERTY_RO("numRecords", NumRecords, "Number of records in the metadata", nullptr, 0),
  };
}

Array<const MethodInfo> KVPStore::getMethods() const
{
  return{
    EP_MAKE_METHOD_EXPLICIT("Insert", InsertMethod, "Insert a record"),
    EP_MAKE_METHOD(Remove, "Remove a record"),
    EP_MAKE_METHOD(Exists, "Check if a record exists"),
    EP_MAKE_METHOD(Get, "Get a record"),
  };
}

} // end namespace ep
