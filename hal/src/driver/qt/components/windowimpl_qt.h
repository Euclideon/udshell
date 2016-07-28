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
    pInstance->ep::Component::getImpl<ep::ComponentImpl>()->SetUserData(pData);

    // register the window with the kernel
    if (static_cast<QtKernel*>(GetKernel())->registerWindow((QQuickWindow*)pInstance->getUserData()) != ep::Result::Success)
      EPTHROW_ERROR(ep::Result::Failure, "Unable to register Window component with Kernel");
  }

  void SetTopLevelUI(ep::UIComponentRef spUIComponent) override final
  {
    QQuickWindow *pQtWindow = (QQuickWindow*)pInstance->getUserData();

    // if there's an existing top level ui, then detach
    foreach(QQuickItem *pChild, pQtWindow->contentItem()->childItems())
      pChild->setParentItem(nullptr);

    if (!spUIComponent)
    {
      spTopLevelUI = nullptr;
    }
    else
    {
      QQuickItem *pUI = (QQuickItem*)(spUIComponent->getUserData());
      EPTHROW_IF(!pUI, ep::Result::InvalidArgument, "The top level UI component does not contain a valid UI");

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
    getImpl<QtWindowImpl>()->spQObject = ep::shared_pointer_cast<QObjectComponent>(_spInstance);
  }

  /*const ep::PropertyDesc *GetPropertyDesc(ep::String _name, ep::EnumerateFlags enumerateFlags = 0) const override final
  {
    return getImpl<QtWindowImpl>()->spQObject->GetPropertyDesc(_name, enumerateFlags);
  }
  const ep::MethodDesc *GetMethodDesc(ep::String _name, ep::EnumerateFlags enumerateFlags = 0) const override final
  {
    return getImpl<QtWindowImpl>()->spQObject->GetMethodDesc(_name, enumerateFlags);
  }
  const ep::EventDesc *GetEventDesc(ep::String _name, ep::EnumerateFlags enumerateFlags = 0) const override final
  {
    return getImpl<QtWindowImpl>()->spQObject->GetEventDesc(_name, enumerateFlags);
  }
  const ep::StaticFuncDesc *GetStaticFuncDesc(ep::String _name, ep::EnumerateFlags enumerateFlags = 0) const override final
  {
    return getImpl<QtWindowImpl>()->spQObject->GetStaticFuncDesc(_name, enumerateFlags);
  }*/
};

} // namespace qt

#endif // EP_WINDOWIMPL_QT_H
