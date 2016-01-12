#pragma once
#if !defined(_EP_IACTIVITY_HPP)
#define _EP_IACTIVITY_HPP

#include "ep/cpp/variant.h"

namespace ep {

// TODO uncomment this and change get and set to use UIComponent when its publicised.
//SHARED_CLASS(UIComponent);

class IActivity
{
public:
  virtual ComponentRef GetUI() const = 0;
  virtual void SetUI(ComponentRef ui) = 0;

  virtual void Activate() = 0;
  virtual void Deactivate() = 0;

  virtual Variant Save() const = 0;
};

} // namespace ep

#endif // _EP_IACTIVITY_HPP
