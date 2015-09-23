#include "text.h"

namespace ud
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

} // namespace ud
