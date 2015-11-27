#pragma once
#ifndef EP_PLUGIN_LOADER_H
#define EP_PLUGIN_LOADER_H

#include "components/component.h"

namespace kernel {

PROTOTYPE_COMPONENT(PluginLoader);

class PluginLoader : public Component
{
  EP_DECLARE_COMPONENT(PluginLoader, Component, EPKERNEL_PLUGINVERSION, "PluginLoader desc...")
public:

  virtual Slice<const String> GetSupportedExtensions() const { return nullptr; }
  virtual bool LoadPlugin(String filename) { return false; }

protected:
  PluginLoader(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, InitParams initParams)
    : Component(pType, pKernel, uid, initParams)
  {}
};

} //namespace kernel

#endif // EP_PLUGIN_LOADER_H
