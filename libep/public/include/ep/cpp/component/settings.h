#pragma once
#ifndef EP_SETTINGS_H
#define EP_SETTINGS_H

//#include "ep/cpp/component/settings.h"
#include "ep/cpp/internal/i/isettings.h"

#include "ep/cpp/component/resource/resource.h"

namespace ep
{

SHARED_CLASS(Settings);

class Settings : public Resource
{
  EP_DECLARE_COMPONENT_WITH_IMPL(ep, Settings, ISettings, Component, EPKERNEL_PLUGINVERSION, "Manage user settings", 0)
public:
  void SaveSettings() { pImpl->SaveSettings(); }

  virtual void SetValue(SharedString nameSpace, SharedString key, Variant value) { pImpl->SetValue(nameSpace, key, value); }
  virtual const Variant GetValue (SharedString nameSpace, SharedString key) { return pImpl->GetValue(nameSpace, key); }

protected:
  Settings(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
  : Resource(pType, pKernel, uid, initParams)
  {
    pImpl = createImpl(initParams);
  }

private:
  Array<const MethodInfo> getMethods() const;
};

} //namespace ep

#endif // EP_SETTINGS_H
