#pragma once
#ifndef EP_VIEWPORTIMPL_QT_H
#define EP_VIEWPORTIMPL_QT_H

#include "components/viewportimpl.h"
#include "driver/qt/ui/renderview_qt.h"

namespace qt {

class QtViewportImpl : public ep::ViewportImpl
{
public:
  QtViewportImpl(Component *pInstance, Variant::VarMap initParams)
    : ep::ViewportImpl(pInstance, initParams)
  {
  }

  void SetView(const ViewRef &_spView) override final
  {
    QQuickItem *pRootItem = (QQuickItem*)pInstance->GetUserData();
    QList<qt::QtRenderView *> renderViews = pRootItem->findChildren<qt::QtRenderView *>();
    EPTHROW_IF(renderViews.size() != 1, epR_Failure, "Viewport component must contain only one RenderView QML item");

    LogDebug(2, "Attaching View Component '{0}' to Viewport", spView->GetUid());
    renderViews.first()->AttachView(spView);
  }
};

} // namespace qt

#endif // EP_VIEWPORTIMPL_QT_H
