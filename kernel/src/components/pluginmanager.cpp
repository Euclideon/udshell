#include "components/pluginmanager.h"
#include "kernel.h"

namespace kernel {

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

} // namespace kernel
