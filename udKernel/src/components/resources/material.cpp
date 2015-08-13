#include "material.h"

namespace ud
{

ComponentDesc Material::descriptor =
{
  &Resource::descriptor, // pSuperDesc

  UDSHELL_APIVERSION, // udVersion
  UDSHELL_PLUGINVERSION, // pluginVersion

  "material", // id
  "Material", // displayName
  "Material resource", // description
};

} // namespace ud
