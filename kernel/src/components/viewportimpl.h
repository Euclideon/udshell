#pragma once
#ifndef EPVIEWPORTIMPL_H
#define EPVIEWPORTIMPL_H

#include "ep/cpp/component/viewport.h"
#include "ep/cpp/internal/i/iviewport.h"
#include "components/uicomponentimpl.h"

namespace ep {

class ViewportImpl : public BaseImpl<Viewport, IViewport>
{
public:
  ViewportImpl(Component *pInstance, Variant::VarMap initParams)
    : ImplSuper(pInstance)
  {
    // check if we passed in a view, otherwise create a default one
    Variant *pVar = initParams.Get("view");
    if (pVar)
    {
      spView = pVar->as<ViewRef>();
    }
    else
    {
      LogDebug(2, "Creating internal View component");
      spView = GetKernel()->CreateComponent<View>();
    }
  }

  ViewRef GetView() const override final { return spView; }

  ViewRef spView = nullptr;
};

class ViewportGlue final : public Viewport
{
public:
  ViewportGlue(const ComponentDesc *_pType, Kernel *_pKernel, SharedString _uid, ComponentRef _spInstance, Variant::VarMap initParams)
    : Viewport(_pType, _pKernel, _uid, initParams), spInstance(_spInstance)
  {
  }

protected:
  ComponentRef spInstance;
};

} // namespace ep

#endif // EPVIEWPORTIMPL_H
