#pragma once
#ifndef EP_UI_QT_H
#define EP_UI_QT_H

#include "component_qt.h"

#include <QQuickItem>
#include <QQuickWindow>

namespace qt {

// This shim class wraps an ep::UIComponent in a QObject that can be accessible from QML
class QtEPUIComponent : public QtEPComponent
{
  Q_OBJECT

public:
  QtEPUIComponent() {}
  QtEPUIComponent(const ep::ComponentRef &spComponent) : QtEPComponent(spComponent) { Setup(); }
  QtEPUIComponent(const QtEPComponent &val) : QtEPComponent(val) { Setup(); }
  ~QtEPUIComponent()
  {
    if (pQuickItem)
      QObject::disconnect(pQuickItem, &QQuickItem::windowChanged, this, &QtEPUIComponent::parentWindowChanged);
  }

  virtual void Done() override
  {
    // we need to call setup from here since Done will only be called on the QML defined "thisComponent"
    // this ensures we've finished loading the UIComponent's qml
    Setup();

    QtEPComponent::Done();
  }

  // QML exposed methods
  Q_INVOKABLE QQuickWindow *parentWindow() const { return pQuickItem->window(); }

signals:
  void parentWindowChanged(QQuickWindow *window);

protected:
  template <typename T> friend struct internal::BuildShimHelper;

  // this constructor ensures the QtEPComponent only holds a weak pointer to its ep::Component
  // currently only used to define the QML "thisComponent" since a SharedPtr will result in a circular reference
  QtEPUIComponent(ep::Component *pComp) : QtEPComponent(pComp) {}

  void Setup()
  {
    if (pComponent)
    {
      pQuickItem = (QQuickItem*)pComponent->GetUserData();
      QObject::connect(pQuickItem, &QQuickItem::windowChanged, this, &QtEPUIComponent::parentWindowChanged);
    }
  }

  QQuickItem *pQuickItem = nullptr;
};

} // namespace qt

#endif // EP_UI_QT_H
