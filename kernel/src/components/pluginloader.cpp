#include "components/pluginloader.h"

namespace ep {

ComponentDesc PluginLoader::descriptor =
{
  &Component::descriptor, // pSuperDesc

  EPSHELL_APIVERSION, // epVersion
  EPSHELL_PLUGINVERSION, // pluginVersion

  "pluginloader", // id
  "Plugin Loader", // displayName
  "Loads plugins", // description

  //Slice<CPropertyDesc>(props, EPARRAYSIZE(props)), // properties
  //Slice<CMethodDesc>(methods, EPARRAYSIZE(methods)), // methods
  nullptr, // properties
  nullptr, // methods
  nullptr, // events
};

} // namespace ep
