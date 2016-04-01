#pragma once
#ifndef QMLBINDINGS_QT_H
#define QMLBINDINGS_QT_H

#include "ep/cpp/component/component.h"
#include "ep/cpp/component/uicomponent.h"
#include "driver/qt/epkernel_qt.h"

#include <QQuickItem>
#include <QQuickWindow>

namespace qt {

// forward declare
class QtEPComponent;
class QtFocusManager;

namespace internal {

// ---------------------------------------------------------------------------------------
// INTERNAL

// Helper function to dynamically populate the component descriptor with the QObject's meta data
void PopulateComponentDesc(ep::Component *pComponent, QObject *pObject);

} // namespace internal


// Helper class that gets stored with the Dynamic Component Descriptor of Qt based components
// This is responsible for loading the QML file, and creating both the associated dynamic component and qobject instance
class QmlComponentData : public ep::RefCounted
{
public:
  QmlComponentData(ep::String file, QQmlEngine *pQmlEngine, QQmlComponent::CompilationMode mode = QQmlComponent::PreferSynchronous)
    : qmlComponent(pQmlEngine)
  {
    EPASSERT_THROW(!file.empty(), epR_InvalidArgument, "Must supply file name");

    // create the qml component for the associated script
    qmlComponent.loadUrl(QUrl(QString::fromUtf8(file.ptr, (int)file.length)), mode);
  }

  ep::DynamicComponentRef CreateComponent(ep::KernelRef spKernel, ep::Variant::VarMap initParams = nullptr);
  QObject *CreateInstance(QQmlEngine *pQmlEngine, ep::Component *pGlueComponent);

private:
  QQmlComponent qmlComponent;
};


// ---------------------------------------------------------------------------------------
// SHIM OBJECTS

struct BuildQtEPComponent;

enum QtHasWeakRef_t { QtHasWeakRef };

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

protected:
  friend struct BuildQtEPComponent;

  // this constructor ensures the QtEPComponent only holds a weak pointer to its ep::Component
  // currently only used to define the QML "thisComponent" since a SharedPtr will result in a circular reference
  QtEPComponent(ep::Component *pComp, QtHasWeakRef_t) : QObject(nullptr), pComponent(pComp) {}

  ep::ComponentRef spComponent;
  ep::Component* pComponent; // used to avoid circular references
};


// This shim class wraps an ep::UIComponent in a QObject that can be accessible from QML
class QtEPUIComponent : public QtEPComponent
{
  Q_OBJECT

public:
  QtEPUIComponent() {}
  QtEPUIComponent(const ep::ComponentRef &spComponent) : QtEPComponent(spComponent) {}
  QtEPUIComponent(const QtEPUIComponent &val) : QtEPComponent(val) {}
  ~QtEPUIComponent() {}

  // QML exposed methods
  Q_INVOKABLE QQuickWindow *parentWindow() const { return static_cast<QQuickItem*>(pComponent->GetUserData())->window(); }
  Q_INVOKABLE QQuickItem *item() const { return static_cast<QQuickItem*>(pComponent->GetUserData()); }

protected:
  friend struct BuildQtEPComponent;

  // this constructor ensures the QtEPComponent only holds a weak pointer to its ep::Component
  // currently only used to define the QML "thisComponent" since a SharedPtr will result in a circular reference
  QtEPUIComponent(ep::Component *pComp, QtHasWeakRef_t) : QtEPComponent(pComp, QtHasWeakRef) {}
};


// This shim class wraps our qt::QtKernel specialised kernel in a QObject that can be accessible from QML
class QtKernelQml : public QObject
{
  Q_OBJECT

public:
  QtKernelQml(QtKernel *_pKernel, QObject *pParent = nullptr) : QObject(pParent), pKernel(_pKernel) {}

  Q_PROPERTY(QtFocusManager * focus READ getFocusManager)

  Q_INVOKABLE qt::QtEPComponent *findComponent(const QString &uid) const;
  Q_INVOKABLE qt::QtEPComponent *createComponent(const QString typeId, QVariant initParams = QVariant());
  Q_INVOKABLE qt::QtEPComponent *getCommandManager() const;

  Q_INVOKABLE qt::QtEPComponent *getLua() const;
  Q_INVOKABLE qt::QtEPComponent *getStdOutBroadcaster() const;
  Q_INVOKABLE qt::QtEPComponent *getStdErrBroadcaster() const;
  Q_INVOKABLE void exec(QString str);

private:
  QtFocusManager *getFocusManager() const;

  QtKernel *pKernel;
};


// factory class to build a qml shim component from a component
struct BuildQtEPComponent
{
  // create a qml shim component that has a shared ref to the ep::component
  static QtEPComponent *Create(const ep::ComponentRef &spComponent) { return CreateInternal(spComponent); }

  // create a qml shim component that has a weak ref to the ep::component - used to avoid circular deps
  static QtEPComponent *CreateWeak(ep::Component *pThis) { return CreateInternal(pThis, QtHasWeakRef); }

private:
  template <typename Component, typename ...Args>
  static QtEPComponent *CreateInternal(Component pComponent, Args ...args)
  {
    if (!pComponent)
      return nullptr;
    if (pComponent->IsType("ep.uicomponent"))
      return new QtEPUIComponent(pComponent, args...);
    return new QtEPComponent(pComponent, args...);
  }
};

} // namespace qt

#endif  // QMLBINDINGS_QT_H
