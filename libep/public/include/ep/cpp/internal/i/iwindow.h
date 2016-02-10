#pragma once
#if !defined(_EP_IWINDOW_HPP)
#define _EP_IWINDOW_HPP

#include "ep/cpp/component/component.h"

namespace ep {

SHARED_CLASS(UIComponent);

class IWindow
{
public:
  virtual void SetTopLevelUI(UIComponentRef spUIComponent) = 0;
  virtual UIComponentRef GetTopLevelUI() const = 0;

protected:
  virtual void InitComplete() = 0;
};

} // namespace ep

#endif // _EP_IWINDOW_HPP
