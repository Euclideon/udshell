#include "resource.h"
#include "metadata.h"
#include "kernel.h"

namespace ep
{

static CPropertyDesc props[] =
{
  {
    {
      "metadata", // id
      "Metadata", // displayName
      "Get the metadata object", // description
    },
    &Resource::GetMetadata,
    nullptr
  }
};
ComponentDesc Resource::descriptor =
{
  &Component::descriptor, // pSuperDesc

  EPSHELL_APIVERSION, // epVersion
  EPSHELL_PLUGINVERSION, // pluginVersion

  "resource", // id
  "Resource", // displayName
  "Base resource", // description

  epSlice<CPropertyDesc>(props, UDARRAYSIZE(props)), // properties
};

MetadataRef Resource::GetMetadata() const
{
  if (!metadata)
    (MetadataRef&)metadata = pKernel->CreateComponent<Metadata>();
  return metadata;
}

} // namespace ep
