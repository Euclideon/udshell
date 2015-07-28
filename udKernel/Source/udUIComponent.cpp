#include "udUIComponent.h"

namespace udKernel
{

const ComponentDesc UIComponent::descriptor =
{
  &Component::descriptor, // pSuperDesc

  UDSHELL_APIVERSION, // udVersion
  UDSHELL_PLUGINVERSION, // pluginVersion

  "ui",                // id
  "udUIComponent",     // displayName
  "Is a ui component", // description

  [](){ return udR_Success; },  // pInit
  UIComponent::Create        // pCreateInstance
};

} // namespace udKernel
