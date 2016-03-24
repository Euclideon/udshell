#pragma once
#ifndef EP_WINDOWIMPL_QT_H
#define EP_WINDOWIMPL_QT_H

#include "components/windowimpl.h"
#include "driver/qt/components/qobjectcomponent_qt.h"

#include <QQuickWindow>
#include <QQuickItem>

namespace qt {

class QtWindowImpl : public ep::WindowImpl
{
public:
  QtWindowImpl(ep::Component *pInstance, ep::Variant::VarMap initParams)
    : ep::WindowImpl(pInstance, initParams)
  {
  }

  void SetTopLevelUI(ep::UIComponentRef spUIComponent) override final
  {
    QQuickWindow *pQtWindow = (QQuickWindow*)pInstance->GetUserData();

    // if there's an existing top level ui, then detach
    foreach(QQuickItem *pChild, pQtWindow->contentItem()->childItems())
      pChild->setParentItem(nullptr);

    if (!spUIComponent)
    {
      spTopLevelUI = nullptr;
    }
    else
    {
      QQuickItem *pUI = (QQuickItem*)(spUIComponent->GetUserData());
      EPTHROW_IF(!pUI, epR_InvalidArgument, "The top level UI component does not contain a valid UI");

      pUI->setParentItem(pQtWindow->contentItem());
      spTopLevelUI = spUIComponent;
    }
  }
};

} // namespace qt

#endif // EP_WINDOWIMPL_QT_H
