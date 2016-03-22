#pragma once
#if !defined(_EP_IVIEWPORT_HPP)
#define _EP_IVIEWPORT_HPP

#include "ep/cpp/component/component.h"

namespace ep {

SHARED_CLASS(View);

class IViewport
{
public:
  virtual ViewRef GetView() const = 0;
  virtual void SetView(const ViewRef &spView) = 0;
};

} // namespace ep

#endif // _EP_IVIEWPORT_HPP
