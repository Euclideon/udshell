#pragma once
#ifndef EP_NATIVE_PLUGIN_LOADER_H
#define EP_NATIVE_PLUGIN_LOADER_H

#include "components/pluginloader.h"

namespace ep {

SHARED_CLASS(NativePluginLoader);

class NativePluginLoader : public PluginLoader
{
  EP_DECLARE_COMPONENT(ep, NativePluginLoader, PluginLoader, EPKERNEL_PLUGINVERSION, "Loads native plugins", 0)
public:

  Slice<const String> getSupportedExtensions() const override;
  bool loadPlugin(String filename) override;

protected:
  NativePluginLoader(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
    : PluginLoader(pType, pKernel, uid, initParams)
  {}
};

} //namespace ep

#endif // EP_NATIVE_PLUGIN_LOADER_H
