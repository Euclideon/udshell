#include "resourceimpl.h"
#include "metadata.h"
#include "ep/cpp/kernel.h"

namespace ep {

Array<const PropertyInfo> Resource::GetProperties() const
{
  return{
    EP_MAKE_PROPERTY_RO(Metadata, "Number of records in the metadata", nullptr, 0),
  };
}
Array<const MethodInfo> Resource::GetMethods() const
{
  return{
    EP_MAKE_METHOD(Save, "Save the state of the Resource"),
  };
}
Array<const EventInfo> Resource::GetEvents() const
{
  return{
    EP_MAKE_EVENT(Changed, "The Resource has been modified"),
  };
}

// TODO Change ComponentRef to Metadata once Metadata made public
ComponentRef ResourceImpl::GetMetadata() const
{
  if (!metadata)
    (MetadataRef&)metadata = GetKernel()->CreateComponent<Metadata>();
  return metadata;
}

} // namespace ep
