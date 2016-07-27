#include "components/pluginmanager.h"
#include "ep/cpp/kernel.h"

namespace ep {

Array<const MethodInfo> PluginManager::getMethods() const
{
  return{
    EP_MAKE_METHOD(RegisterPluginLoader, "Register a PluginLoader"),
    EP_MAKE_METHOD(LoadPlugin, "Load a plugin with the given filename"),
  };
}

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
