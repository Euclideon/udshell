#pragma once
#if !defined(_EP_IVIEWPORT_HPP)
#define _EP_IVIEWPORT_HPP

namespace ep {

// TODO: Add view methods when View is impl'd
//SHARED_CLASS(View);

class IViewport
{
public:
  //virtual ViewRef GetView() const = 0;
  //virtual void SetView(ViewRef view) = 0;

protected:
  virtual void InitComplete() = 0;
};

} // namespace ep

#endif // _EP_IVIEWPORT_HPP
