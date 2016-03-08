#include "components/pluginmanager.h"
#include "ep/cpp/kernel.h"

namespace ep {

PluginManager::PluginManager(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
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
