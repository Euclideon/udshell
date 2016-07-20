#include "resourceimpl.h"
#include "ep/cpp/component/resource/metadata.h"
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
    EP_MAKE_METHOD(Clone, "Make a shallow clone of this object"),
  };
}
Array<const EventInfo> Resource::GetEvents() const
{
  return{
    EP_MAKE_EVENT(Changed, "The Resource has been modified"),
  };
}

MetadataRef ResourceImpl::GetMetadata() const
{
  if (!metadata)
    (MetadataRef&)metadata = GetKernel()->CreateComponent<Metadata>();
  return metadata;
}

} // namespace ep
