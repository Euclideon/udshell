#pragma once
#ifndef EPVIEWPORTIMPL_H
#define EPVIEWPORTIMPL_H

#include "ep/cpp/component/viewport.h"
#include "ep/cpp/internal/i/iviewport.h"

namespace ep {

class ViewportImpl : public BaseImpl<Viewport, IViewport>
{
public:
  ViewportImpl(Component *pInstance, Variant::VarMap initParams)
    : Super(pInstance)
  {
    CreateInternal(initParams);
  }

private:
  void InitComplete() override final {}

  void CreateInternal(Variant::VarMap initParams);

  ViewRef spView = nullptr;
};

} // namespace ep

#endif // EPVIEWPORTIMPL_H
