#include "ep/cpp/component/resource/kvpstore.h"

namespace ep {

Array<const PropertyInfo> KVPStore::getProperties() const
{
  return{
    EP_MAKE_PROPERTY_RO("numRecords", numRecords, "Number of records in the metadata", nullptr, 0),
  };
}

Array<const MethodInfo> KVPStore::getMethods() const
{
  return{
    EP_MAKE_METHOD_EXPLICIT("insert", insertMethod, "Insert a record"),
    EP_MAKE_METHOD(remove, "Remove a record"),
    EP_MAKE_METHOD(exists, "Check if a record exists"),
    EP_MAKE_METHOD(get, "Get a record"),
  };
}

} // end namespace ep
