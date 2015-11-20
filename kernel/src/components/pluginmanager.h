#pragma once
#ifndef EP_PLUGIN_MANAGER_H
#define EP_PLUGIN_MANAGER_H

#include "components/component.h"
#include "components/pluginloader.h"

namespace kernel {

PROTOTYPE_COMPONENT(PluginManager);

class PluginManager : public Component
{
  EP_DECLARE_COMPONENT(PluginManager, Component, EPKERNEL_PLUGINVERSION, "PluginManager desc...")
public:

  void RegisterPluginLoader(PluginLoaderRef spLoader);
  bool LoadPlugin(String filename);

protected:
  PluginManager(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, InitParams initParams);

  Array<PluginLoaderRef> loaders;
};

} //namespace kernel

#endif // EP_PLUGIN_MANAGER_H
