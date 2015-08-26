#include "uicomponent.h"

namespace ud
{

ComponentDesc UIComponent::descriptor =
{
  &Component::descriptor, // pSuperDesc

  UDSHELL_APIVERSION, // udVersion
  UDSHELL_PLUGINVERSION, // pluginVersion

  "ui",                // id
  "UIComponent",     // displayName
  "Is a ui component", // description

  nullptr, // properties
  nullptr, // methods
  nullptr, // events

  nullptr, // pInit
  UIComponent::Create, // pCreate
};

} // namespace ud
