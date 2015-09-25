#include "text.h"

namespace ep
{

ComponentDesc Text::descriptor =
{
  &Buffer::descriptor, // pSuperDesc

  EPSHELL_APIVERSION, // epVersion
  EPSHELL_PLUGINVERSION, // pluginVersion

  "text", // id
  "Text", // displayName
  "Text resource", // description
};

} // namespace ep
