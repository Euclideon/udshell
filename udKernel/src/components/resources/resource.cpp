#include "resource.h"

namespace ud
{

ComponentDesc Resource::descriptor =
{
  &Component::descriptor, // pSuperDesc

  UDSHELL_APIVERSION, // udVersion
  UDSHELL_PLUGINVERSION, // pluginVersion

  "resource", // id
  "Resource", // displayName
  "Is a resource", // description
};

} // namespace ud
