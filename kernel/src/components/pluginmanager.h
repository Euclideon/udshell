#pragma once
#ifndef EP_PLUGIN_MANAGER_H
#define EP_PLUGIN_MANAGER_H

#include "components/component.h"
#include "components/pluginloader.h"

namespace ep {

PROTOTYPE_COMPONENT(PluginManager);

class PluginManager : public Component
{
public:
  EP_COMPONENT(PluginManager);

  void RegisterPluginLoader(PluginLoaderRef spLoader);
  bool LoadPlugin(String filename);

protected:
  PluginManager(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, InitParams initParams);

  Array<PluginLoaderRef> loaders;
};

} //namespace ep

#endif // EP_PLUGIN_MANAGER_H
