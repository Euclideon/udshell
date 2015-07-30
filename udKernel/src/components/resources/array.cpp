#include "array.h"

namespace ud
{

ComponentDesc ArrayBuffer::descriptor =
{
  &Resource::descriptor, // pSuperDesc

  UDSHELL_APIVERSION, // udVersion
  UDSHELL_PLUGINVERSION, // pluginVersion

  "array", // id
  "Array", // displayName
  "Array resource", // description
};

} // namespace ud
