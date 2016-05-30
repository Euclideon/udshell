#pragma once
#ifndef EP_SETTINGS_H
#define EP_SETTINGS_H

#include "ep/cpp/component/component.h"

namespace ep
{

SHARED_CLASS(Settings);

class Settings : public Component
{
  EP_DECLARE_COMPONENT(ep, Settings, Component, EPKERNEL_PLUGINVERSION, "Manage user settings", 0)
public:

  void SaveSettings();

  void SetValue(SharedString nameSpace, SharedString key, Variant value);
  Variant GetValue(SharedString nameSpace, SharedString key);

protected:
  Settings(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams);

  AVLTree<SharedString, Variant> settings;

  SharedString srcString;

  void ParseSettings(Variant node);
  void ParsePluginSettings(Variant node);

  Array<const MethodInfo> GetMethods() const;
};

} //namespace ep

#endif // EP_SETTINGS_H
