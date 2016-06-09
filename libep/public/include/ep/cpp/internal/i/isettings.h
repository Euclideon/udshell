#pragma once
#if !defined(_EP_ISETTINGS_HPP)
#define _EP_ISETTINGS_HPP

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

#endif // _EP_ISETTINGS_HPP
