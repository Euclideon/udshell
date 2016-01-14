#include "resourceimpl.h"
#include "metadata.h"
#include "kernel.h"

namespace ep {

// TODO Change ComponentRef to Metadata once Metadata made public
ComponentRef ResourceImpl::GetMetadata() const
{
  if (!metadata)
    (MetadataRef&)metadata = GetKernel()->CreateComponent<Metadata>();
  return metadata;
}

} // namespace ep
