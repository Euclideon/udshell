#include "model.h"

namespace ud
{

ComponentDesc Model::descriptor =
{
  &Resource::descriptor, // pSuperDesc

  UDSHELL_APIVERSION, // udVersion
  UDSHELL_PLUGINVERSION, // pluginVersion

  "model", // id
  "Model", // displayName
  "Model resource", // description
};

} // namespace ud
