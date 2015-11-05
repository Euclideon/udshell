#pragma once
#ifndef EP_PLUGIN_LOADER_H
#define EP_PLUGIN_LOADER_H

#include "components/component.h"

namespace ep {

PROTOTYPE_COMPONENT(PluginLoader);

class PluginLoader : public Component
{
public:
  EP_COMPONENT(PluginLoader);

  virtual Slice<const String> GetSupportedExtensions() const { return nullptr; }
  virtual bool LoadPlugin(String filename) { return false; }

protected:
  PluginLoader(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, InitParams initParams)
    : Component(pType, pKernel, uid, initParams)
  {}
};

} //namespace ep

#endif // EP_PLUGIN_LOADER_H
