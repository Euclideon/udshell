#include "shader.h"

namespace ud
{

ComponentDesc Shader::descriptor =
{
  &Resource::descriptor, // pSuperDesc

  UDSHELL_APIVERSION, // udVersion
  UDSHELL_PLUGINVERSION, // pluginVersion

  "shader", // id
  "Shader", // displayName
  "Shader resource", // description
};

} // namespace ud
