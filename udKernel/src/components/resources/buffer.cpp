#include "buffer.h"

namespace ud
{

ComponentDesc Buffer::descriptor =
{
  &Resource::descriptor, // pSuperDesc

  UDSHELL_APIVERSION, // udVersion
  UDSHELL_PLUGINVERSION, // pluginVersion

  "buffer", // id
  "Buffer", // displayName
  "Buffer resource", // description
};

} // namespace ud
