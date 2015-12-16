#include "resource.h"
#include "metadata.h"
#include "kernel.h"

namespace ep {

MetadataRef Resource::GetMetadata() const
{
  if (!metadata)
    (MetadataRef&)metadata = GetKernel().CreateComponent<Metadata>();
  return metadata;
}

} // namespace ep
