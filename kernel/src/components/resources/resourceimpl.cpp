#include "resourceimpl.h"
#include "ep/cpp/component/resource/metadata.h"
#include "ep/cpp/kernel.h"

namespace ep {

Array<const PropertyInfo> Resource::getProperties() const
{
  return{
    EP_MAKE_PROPERTY_RO("metadata", getMetadata, "Number of records in the metadata", nullptr, 0),
  };
}
Array<const MethodInfo> Resource::getMethods() const
{
  return{
    EP_MAKE_METHOD(save, "Save the state of the Resource"),
    EP_MAKE_METHOD(clone, "Make a shallow clone of this object"),
  };
}
Array<const EventInfo> Resource::getEvents() const
{
  return{
    EP_MAKE_EVENT(changed, "The Resource has been modified"),
  };
}

MetadataRef ResourceImpl::GetMetadata() const
{
  if (!metadata)
    (MetadataRef&)metadata = getKernel()->createComponent<Metadata>();
  return metadata;
}

} // namespace ep
