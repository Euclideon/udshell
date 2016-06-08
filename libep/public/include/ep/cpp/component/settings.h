#pragma once
#ifndef EP_SETTINGS_H
#define EP_SETTINGS_H

#include "ep/cpp/internal/i/isettings.h"

#include "ep/cpp/component/resource/resource.h"

namespace ep
{

  SHARED_CLASS(Settings);

  class Settings : public Resource
  {
    EP_DECLARE_COMPONENT(ep, Settings, Component, EPKERNEL_PLUGINVERSION, "Manage user settings", 0)
  public:
    virtual void SaveSettings();

    virtual void SetValue(SharedString nameSpace, SharedString key, Variant value);
    virtual Variant GetValue(SharedString nameSpace, SharedString key);

  protected:
    Settings(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams);

  private:
    Array<const MethodInfo> GetMethods() const;
  };

} //namespace ep

#endif // EP_SETTINGS_H
