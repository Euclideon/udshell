#include "ep/cpp/component/resource/kvpstore.h"

namespace ep {

Array<const PropertyInfo> KVPStore::GetProperties() const
{
  return{
    EP_MAKE_PROPERTY_EXPLICIT("NumRecords", "Number of records in the metadata", EP_MAKE_GETTER(NumRecords), nullptr, nullptr, 0),
  };
}

Array<const MethodInfo> KVPStore::GetMethods() const
{
  return{
    EP_MAKE_METHOD_EXPLICIT("Insert", InsertMethod, "Insert a record"),
    EP_MAKE_METHOD(Remove, "Remove a record"),
    EP_MAKE_METHOD(Exists, "Check if a record exists"),
    EP_MAKE_METHOD(Get, "Get a record"),
  };
}

} // end namespace ep
