#pragma once
#ifndef EP_PLUGIN_LOADER_H
#define EP_PLUGIN_LOADER_H

#include "ep/cpp/component.h"

namespace ep {

SHARED_CLASS(PluginLoader);

class PluginLoader : public Component
{
  EP_DECLARE_COMPONENT(PluginLoader, Component, EPKERNEL_PLUGINVERSION, "PluginLoader desc...")
public:

  virtual Slice<const String> GetSupportedExtensions() const { return nullptr; }
  virtual bool LoadPlugin(String filename) { return false; }

protected:
  PluginLoader(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
    : Component(pType, pKernel, uid, initParams)
  {}
};

} //namespace ep

#endif // EP_PLUGIN_LOADER_H
