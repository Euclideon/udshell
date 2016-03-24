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

  void PostInit(void *pData) override final
  {
    pInstance->ep::Component::GetImpl<ep::ComponentImpl>()->SetUserData(pData);

    // register the window with the kernel
    if (static_cast<QtKernel*>(GetKernel())->RegisterWindow((QQuickWindow*)pInstance->GetUserData()) != epR_Success)
      EPTHROW_ERROR(epR_Failure, "Unable to register Window component with Kernel");
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

  QObjectComponentRef spQObject = nullptr;
};

class WindowGlue final : public ep::Window
{
public:
  WindowGlue(const ep::ComponentDesc *_pType, ep::Kernel *_pKernel, ep::SharedString _uid, ep::ComponentRef _spInstance, ep::Variant::VarMap initParams)
    : Window(_pType, _pKernel, _uid, initParams)
  {
    GetImpl<QtWindowImpl>()->spQObject = ep::shared_pointer_cast<QObjectComponent>(_spInstance);
  }
};

} // namespace qt

#endif // EP_WINDOWIMPL_QT_H
