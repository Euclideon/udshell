#include "uicomponent.h"

namespace ud
{

ComponentDesc UIComponent::descriptor =
{
  &Component::descriptor, // pSuperDesc

  UDSHELL_APIVERSION, // udVersion
  UDSHELL_PLUGINVERSION, // pluginVersion

  "ui",                // id
  "udUIComponent",     // displayName
  "Is a ui component", // description
};

} // namespace ud
