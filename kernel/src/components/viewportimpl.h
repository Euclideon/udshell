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

  ViewRef GetView() const override final { return spView; }
  void SetView(ViewRef _spView) override final { EPASSERT(false, "TODO: implement this - need to reload the ui!!"); }

private:
  void InitComplete() override final {}

  void CreateInternal(Variant::VarMap initParams);

  ViewRef spView = nullptr;
};

} // namespace ep

#endif // EPVIEWPORTIMPL_H
