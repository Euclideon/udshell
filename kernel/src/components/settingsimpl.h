#pragma once
#ifndef EPSETTINGSIMPL_H
#define EPSETTINGSIMPL_H

#include "ep/cpp/component/settings.h"
#include "ep/cpp/internal/i/isettings.h"

#include "ep/cpp/component/resource/resource.h"

namespace ep
{

SHARED_CLASS(Settings);

class SettingsImpl : public BaseImpl<Settings, ISettings>
{
public:
  SettingsImpl(Component *pInstance, Variant::VarMap initParams);

  void SaveSettings() override final;

  void SetValue(SharedString nameSpace, SharedString key, Variant value) override final;
  Variant GetValue(SharedString nameSpace, SharedString key) override final;

protected:
  AVLTree<SharedString, Variant> settings;

  SharedString srcString;

  void ParseSettings(Variant node);
  void ParsePluginSettings(Variant node);

  Array<const MethodInfo> GetMethods() const;
};

} //namespace ep

#endif // EPSETTINGSIMPL_H
