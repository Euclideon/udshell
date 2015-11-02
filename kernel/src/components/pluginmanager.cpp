#include "components/pluginmanager.h"
#include "kernel.h"

namespace ep {

/*
static CMethodDesc methods[] =
{

};
static CPropertyDesc props[] =
{

};
*/

ComponentDesc PluginManager::descriptor =
{
  &Component::descriptor, // pSuperDesc

  EPSHELL_APIVERSION, // epVersion
  EPSHELL_PLUGINVERSION, // pluginVersion

  "pluginmanager", // id
  "Plugin Manager", // displayName
  "Manages plugins", // description

  //Slice<CPropertyDesc>(props, EPARRAYSIZE(props)), // properties
  //Slice<CMethodDesc>(methods, EPARRAYSIZE(methods)), // methods
  nullptr, // properties
  nullptr, // methods
  nullptr, // events
};

PluginManager::PluginManager(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, InitParams initParams)
  : Component(pType, pKernel, uid, initParams)
{

}

void PluginManager::RegisterPluginLoader(PluginLoaderRef spLoader)
{
  loaders.pushBack(spLoader);
}

bool PluginManager::LoadPlugin(String filename)
{
  String ext = filename.getRightAtLast('.');
  for (auto &loader : loaders)
  {
    if (loader->GetSupportedExtensions().exists(ext))
    {
      return loader->LoadPlugin(filename);
    }
  }
  return false;
}

} // namespace ep
