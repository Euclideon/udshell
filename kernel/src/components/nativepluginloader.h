#pragma once
#ifndef EP_NATIVE_PLUGIN_LOADER_H
#define EP_NATIVE_PLUGIN_LOADER_H

#include "components/pluginloader.h"

namespace kernel {

PROTOTYPE_COMPONENT(NativePluginLoader);

class NativePluginLoader : public PluginLoader
{
public:
  EP_COMPONENT(NativePluginLoader);

  Slice<const String> GetSupportedExtensions() const override;
  bool LoadPlugin(String filename) override;

protected:
  NativePluginLoader(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, InitParams initParams)
    : PluginLoader(pType, pKernel, uid, initParams)
  {}
};

} //namespace kernel

#endif // EP_NATIVE_PLUGIN_LOADER_H
