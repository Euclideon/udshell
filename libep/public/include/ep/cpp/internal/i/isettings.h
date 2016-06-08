#pragma once
#ifndef EP_SETTINGS_H
#define EP_SETTINGS_H

#include "ep/cpp/component/resource/resource.h"

namespace ep
{

SHARED_CLASS(Settings);

class ISettings
{
public:
  virtual void SaveSettings() = 0;

  virtual void SetValue(SharedString nameSpace, SharedString key, Variant value) = 0;
  virtual Variant GetValue(SharedString nameSpace, SharedString key) = 0;
};

} //namespace ep

#endif // EP_SETTINGS_H
