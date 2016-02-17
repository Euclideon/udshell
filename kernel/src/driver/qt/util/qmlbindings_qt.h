#pragma once
#ifndef QMLBINDINGS_QT_H
#define QMLBINDINGS_QT_H

#include "ep/cpp/component/component.h"

#include <QQuickItem>
#include <QQuickWindow>

namespace qt {

// forward declare
class QtEPComponent;
class QtKernel;

namespace internal {

// ---------------------------------------------------------------------------------------
// INTERNAL

// Internal helper that creates a QtEPComponent (or derived) shim object containing a weak pointer
template <typename T>
struct BuildShimHelper
{
  epforceinline static QtEPComponent *Create(ep::Component *pComponent)
  {
    return new T(pComponent);
  }
};

// Helper function to dynamically populate the component descriptor with the QObject's meta data
void PopulateComponentDesc(Component *pComponent, QObject *pObject);

} // namespace internal


// ---------------------------------------------------------------------------------------
// SHIM OBJECTS

// This shim class wraps an ep::Component in a QObject that can be accessible from QML
class QtEPComponent : public QObject
{
  Q_OBJECT

public:
  QtEPComponent() : QObject(nullptr), pComponent(nullptr) {}
  QtEPComponent(const ep::ComponentRef &spComponent) : QObject(nullptr), spComponent(spComponent) { pComponent = spComponent.ptr(); }
  QtEPComponent(const QtEPComponent &val) : QObject(val.parent()), spComponent(ep::ComponentRef(val.pComponent)), pComponent(val.pComponent) {}
  ~QtEPComponent() {}

  ep::ComponentRef GetComponent() const { return ep::ComponentRef(pComponent); }

  // methods to inspect internals
  //..

public:
  virtual void Done() { emit completed(); }

  Q_INVOKABLE bool isNull() const { return pComponent == nullptr; }

  Q_INVOKABLE QVariant get(const QString &name) const;
  Q_INVOKABLE void set(const QString &name, QVariant val);

  // Ugly but necessary
  Q_INVOKABLE QVariant call(const QString &name) const;
  Q_INVOKABLE QVariant call(const QString &name, QVariant arg0) const;
  Q_INVOKABLE QVariant call(const QString &name, QVariant arg0, QVariant arg1) const;
  Q_INVOKABLE QVariant call(const QString &name, QVariant arg0, QVariant arg1, QVariant arg2) const;
  Q_INVOKABLE QVariant call(const QString &name, QVariant arg0, QVariant arg1, QVariant arg2, QVariant arg3) const;
  Q_INVOKABLE QVariant call(const QString &name, QVariant arg0, QVariant arg1, QVariant arg2, QVariant arg3, QVariant arg4) const;
  Q_INVOKABLE QVariant call(const QString &name, QVariant arg0, QVariant arg1, QVariant arg2, QVariant arg3, QVariant arg4, QVariant arg5) const;
  Q_INVOKABLE QVariant call(const QString &name, QVariant arg0, QVariant arg1, QVariant arg2, QVariant arg3, QVariant arg4, QVariant arg5, QVariant arg6) const;
  Q_INVOKABLE QVariant call(const QString &name, QVariant arg0, QVariant arg1, QVariant arg2, QVariant arg3, QVariant arg4, QVariant arg5, QVariant arg6, QVariant arg7) const;
  Q_INVOKABLE QVariant call(const QString &name, QVariant arg0, QVariant arg1, QVariant arg2, QVariant arg3, QVariant arg4, QVariant arg5, QVariant arg6, QVariant arg7, QVariant arg8) const;
  Q_INVOKABLE QVariant call(const QString &name, QVariant arg0, QVariant arg1, QVariant arg2, QVariant arg3, QVariant arg4, QVariant arg5, QVariant arg6, QVariant arg7, QVariant arg8, QVariant arg9) const;

  Q_INVOKABLE void subscribe(QString eventName, QJSValue func) const;

signals:
  void completed();

protected:
  template <typename T> friend struct internal::BuildShimHelper;

  // this constructor ensures the QtEPComponent only holds a weak pointer to its ep::Component
  // currently only used to define the QML "thisComponent" since a SharedPtr will result in a circular reference
  QtEPComponent(ep::Component* pComp) : QObject(nullptr), pComponent(pComp) {}

  ep::ComponentRef spComponent;
  Component* pComponent; // used to avoid circular references
};


// This shim class wraps an ep::UIComponent in a QObject that can be accessible from QML
class QtEPUIComponent : public QtEPComponent
{
  Q_OBJECT

public:
  QtEPUIComponent() {}
  QtEPUIComponent(const ep::ComponentRef &spComponent) : QtEPComponent(spComponent) { InitQuickItem(); }
  QtEPUIComponent(const QtEPComponent &val) : QtEPComponent(val) { InitQuickItem(); }
  ~QtEPUIComponent()
  {
    if (pQuickItem)
      QObject::disconnect(pQuickItem, &QQuickItem::windowChanged, this, &QtEPUIComponent::parentWindowChanged);
  }

  virtual void Done() override
  {
    // we need to call InitQuickItem() from here since Done() will only be called on the QML defined "thisComponent"
    // this ensures we've finished loading the UIComponent's qml
    InitQuickItem();

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

  void InitQuickItem()
  {
    if (pComponent)
    {
      pQuickItem = (QQuickItem*)pComponent->GetUserData();
      QObject::connect(pQuickItem, &QQuickItem::windowChanged, this, &QtEPUIComponent::parentWindowChanged);
    }
  }

  QQuickItem *pQuickItem = nullptr;
};


// This shim class wraps our qt::QtKernel specialised kernel in a QObject that can be accessible from QML
class QtKernelQml : public QObject
{
  Q_OBJECT

public:
  QtKernelQml(QtKernel *_pKernel, QObject *pParent = nullptr) : QObject(pParent), pKernel(_pKernel) {}

  Q_INVOKABLE qt::QtEPComponent *findComponent(const QString &uid) const;
  Q_INVOKABLE qt::QtEPComponent *createComponent(const QString typeId, QVariantMap initParams = QVariantMap());
  Q_INVOKABLE qt::QtEPComponent *getCommandManager() const;

private:
  QtKernel *pKernel;
};


// factory class to build a qml shim component from a component ref
// performs a lookup, so only use this if the type isn't known at compile time
struct BuildQtEPComponent
{
  static QtEPComponent *Create(const ep::ComponentRef &spComponent);
};

} // namespace qt

#endif  // QMLBINDINGS_QT_H
