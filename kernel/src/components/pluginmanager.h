#pragma once
#ifndef EP_PLUGIN_MANAGER_H
#define EP_PLUGIN_MANAGER_H

#include "component.h"

namespace ep {

PROTOTYPE_COMPONENT(PluginManager);

class PluginManager : public Component
{
public:
  EP_COMPONENT(PluginManager);

  bool LoadPlugin(String filename);

protected:
  PluginManager(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, InitParams initParams);
  ~PluginManager();

//  udAVLTree<udString, ResourceRef> plugins;
};

} //namespace ep

#endif // EP_PLUGIN_MANAGER_H
