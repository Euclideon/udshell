#pragma once
#ifndef QML_PLUGIN_LOADER_QT_H
#define QML_PLUGIN_LOADER_QT_H

#include "components/pluginloader.h"

namespace qt {

class QtKernel;

SHARED_CLASS(QmlPluginLoader);

class QmlPluginLoader : public ep::PluginLoader
{
  EP_DECLARE_COMPONENT(ep, QmlPluginLoader, PluginLoader, EPKERNEL_PLUGINVERSION, "Loads QML plugins", 0)
public:

  ep::Slice<const ep::String> GetSupportedExtensions() const override;
  bool LoadPlugin(ep::String filename) override;

  static ep::Variant::VarMap ParseTypeDescriptor(QtKernel *pQtKernel, ep::String filename);

protected:
  QmlPluginLoader(const ep::ComponentDesc *pType, ep::Kernel *pKernel, ep::SharedString uid, ep::Variant::VarMap initParams)
    : PluginLoader(pType, pKernel, uid, initParams)
  {}
};

} //namespace qt

#endif // QML_PLUGIN_LOADER_QT_H
