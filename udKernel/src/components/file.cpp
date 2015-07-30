#include "components/file.h"

namespace ud
{

ComponentDesc File::descriptor =
{
  &Stream::descriptor, // pSuperDesc

  UDSHELL_APIVERSION, // udVersion
  UDSHELL_PLUGINVERSION, // pluginVersion

  "file", // id
  "File", // displayName
  "File object", // description
};

} // namespace ud
