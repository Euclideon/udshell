#include "components/stream.h"

namespace ud
{

ComponentDesc Stream::descriptor =
{
  &Component::descriptor, // pSuperDesc

  UDSHELL_APIVERSION, // udVersion
  UDSHELL_PLUGINVERSION, // pluginVersion

  "stream", // id
  "Stream", // displayName
  "Data stream", // description
};

} // namespace ud
