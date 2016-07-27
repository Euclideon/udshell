#pragma once
#ifndef EP_PLUGIN_LOADER_H
#define EP_PLUGIN_LOADER_H

#include "ep/cpp/component/component.h"

namespace ep {

SHARED_CLASS(PluginLoader);

class PluginLoader : public Component
{
  EP_DECLARE_COMPONENT(ep, PluginLoader, Component, EPKERNEL_PLUGINVERSION, "PluginLoader desc...", 0)
public:

  virtual Slice<const String> getSupportedExtensions() const { return nullptr; }
  virtual bool loadPlugin(String filename) { return false; }

protected:
  PluginLoader(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
    : Component(pType, pKernel, uid, initParams)
  {}

  Array<const PropertyInfo> getProperties() const
  {
    return{
      EP_MAKE_PROPERTY_RO("supportedExtensions", getSupportedExtensions, "List of file extension strings handled by this PluginLoader", nullptr, 0),
    };
  }

  Array<const MethodInfo> getMethods() const
  {
    return{
      EP_MAKE_METHOD(loadPlugin, "Load a plugin with the given filename"),
    };
  }
};

} //namespace ep

#endif // EP_PLUGIN_LOADER_H
