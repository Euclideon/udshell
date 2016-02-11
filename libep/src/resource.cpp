#include "ep/cpp/component/resource/resource.h"
#include "ep/cpp/component/datasource/datasource.h"
#include "ep/cpp/kernel.h"

namespace ep
{

Array<const PropertyInfo> Resource::GetProperties()
{
  return{
    EP_MAKE_PROPERTY_RO(Metadata, "Number of records in the metadata", nullptr, 0),
    EP_MAKE_PROPERTY_RO(DataSource, "The DataSource containing this Resource", nullptr, 0),
  };
}
Array<const MethodInfo> Resource::GetMethods()
{
  return{
    EP_MAKE_METHOD(Save, "Save the state of the Resource"),
  };
}
Array<const EventInfo> Resource::GetEvents()
{
  return{
    EP_MAKE_EVENT(Changed, "The Resource has been modified"),
  };
}

}
