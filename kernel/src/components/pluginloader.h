#pragma once
#ifndef EP_PLUGIN_LOADER_H
#define EP_PLUGIN_LOADER_H

#include "ep/cpp/component/component.h"

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

  Array<const PropertyInfo> GetProperties()
  {
    return{
      EP_MAKE_PROPERTY_RO(SupportedExtensions, "List of file extension strings handled by this PluginLoader", nullptr, 0),
    };
  }

  static Array<const MethodInfo> GetMethods()
  {
    return{
      EP_MAKE_METHOD(LoadPlugin, "Load a plugin with the given filename"),
    };
  }
};

} //namespace ep

#endif // EP_PLUGIN_LOADER_H
