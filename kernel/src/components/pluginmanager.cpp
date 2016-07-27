#include "components/pluginmanager.h"
#include "ep/cpp/kernel.h"

namespace ep {

Array<const MethodInfo> PluginManager::getMethods() const
{
  return{
    EP_MAKE_METHOD(registerPluginLoader, "Register a PluginLoader"),
    EP_MAKE_METHOD(loadPlugin, "Load a plugin with the given filename"),
  };
}

PluginManager::PluginManager(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
  : Component(pType, pKernel, uid, initParams)
{

}

void PluginManager::registerPluginLoader(PluginLoaderRef spLoader)
{
  loaders.pushBack(spLoader);
}

bool PluginManager::loadPlugin(String filename)
{
  String ext = filename.getRightAtLast('.');
  for (auto &loader : loaders)
  {
    if (loader->getSupportedExtensions().exists(ext))
    {
      return loader->loadPlugin(filename);
    }
  }
  return false;
}

} // namespace ep
