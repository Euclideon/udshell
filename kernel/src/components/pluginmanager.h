#pragma once
#ifndef EP_PLUGIN_MANAGER_H
#define EP_PLUGIN_MANAGER_H

#include "ep/cpp/component/component.h"
#include "components/pluginloader.h"

namespace ep {

SHARED_CLASS(PluginManager);

class PluginManager : public Component
{
  EP_DECLARE_COMPONENT(ep, PluginManager, Component, EPKERNEL_PLUGINVERSION, "PluginManager desc...", 0)
public:

  void registerPluginLoader(PluginLoaderRef spLoader);
  bool loadPlugin(String filename);

protected:
  PluginManager(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams);

  Array<PluginLoaderRef> loaders;

  Array<const MethodInfo> getMethods() const;
};

} //namespace ep

#endif // EP_PLUGIN_MANAGER_H
