#include "text.h"

namespace ep
{

ComponentDesc Text::descriptor =
{
  &Buffer::descriptor, // pSuperDesc

  UDSHELL_APIVERSION, // udVersion
  UDSHELL_PLUGINVERSION, // pluginVersion

  "text", // id
  "Text", // displayName
  "Text resource", // description
};

} // namespace ep
