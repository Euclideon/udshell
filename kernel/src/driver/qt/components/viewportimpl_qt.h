#pragma once
#ifndef EP_VIEWPORTIMPL_QT_H
#define EP_VIEWPORTIMPL_QT_H

#include "components/viewportimpl.h"
#include "driver/qt/ui/renderview_qt.h"
#include "driver/qt/components/uicomponentimpl_qt.h"

namespace qt {

class QtViewportImpl : public ep::ViewportImpl
{
public:
  QtViewportImpl(ep::Component *pInstance, ep::Variant::VarMap initParams)
    : ep::ViewportImpl(pInstance, initParams)
  {
  }

  void SetView(const ep::ViewRef &_spView) override final
  {
    QQuickItem *pRootItem = (QQuickItem*)pInstance->GetUserData();
    QList<qt::QtRenderView *> renderViews = pRootItem->findChildren<qt::QtRenderView *>();
    EPTHROW_IF(renderViews.size() != 1, epR_Failure, "Viewport component must contain only one RenderView QML item");

    LogDebug(2, "Attaching View Component '{0}' to Viewport", spView->GetUid());
    renderViews.first()->AttachView(spView);
  }

  void PostInit(void *pData) override final
  {
    pInstance->Super::PostInit(pData);
    SetView(spView);
  }
};

class ViewportGlue final : public ep::Viewport
{
public:
  ViewportGlue(const ep::ComponentDesc *_pType, ep::Kernel *_pKernel, ep::SharedString _uid, ep::ComponentRef _spInstance, ep::Variant::VarMap initParams)
    : ep::Viewport(_pType, _pKernel, _uid, initParams)
  {
    ep::UIComponent::GetImpl<QtUIComponentImpl>()->spQObject = ep::shared_pointer_cast<QObjectComponent>(_spInstance);
  }
};

} // namespace qt

#endif // EP_VIEWPORTIMPL_QT_H
