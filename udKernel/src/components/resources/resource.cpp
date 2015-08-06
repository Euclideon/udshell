#include "resource.h"
#include "metadata.h"
#include "kernel.h"

namespace ud
{

static PropertyDesc props[] =
{
  {
    "metadata", // id
    "Metadata", // displayName
    "Get the metadata object", // description
    &Resource::GetMetadata,
    nullptr,
    TypeDesc(PropertyType::Resource)
  }
};
ComponentDesc Resource::descriptor =
{
  &Component::descriptor, // pSuperDesc

  UDSHELL_APIVERSION, // udVersion
  UDSHELL_PLUGINVERSION, // pluginVersion

  "resource", // id
  "Resource", // displayName
  "Base resource", // description

  udSlice<PropertyDesc>(props, UDARRAYSIZE(props)), // properties
};

Resource::Resource(const ComponentDesc *pType, Kernel *pKernel, udRCString uid, udInitParams initParams)
  : Component(pType, pKernel, uid, initParams)
{
  metadata = pKernel->CreateComponent<Metadata>();
}

} // namespace ud
