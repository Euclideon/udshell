#pragma once
#if !defined(_EP_IUICOMPONENT_HPP)
#define _EP_IUICOMPONENT_HPP

#include "ep/cpp/component/component.h"

namespace ep {

class IUIComponent
{
public:
  virtual Variant GetUIHandle() const = 0;
  virtual void PostInit(void *pData) = 0;
};

} // namespace ep

#endif // _EP_IUICOMPONENT_HPP
