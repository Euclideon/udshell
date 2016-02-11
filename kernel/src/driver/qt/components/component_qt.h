#pragma once
#ifndef EP_COMPONENT_QT_H
#define EP_COMPONENT_QT_H

#include "../util/typeconvert_qt.h"

#include "components/componentimpl.h"

#include <QVariant>

namespace qt {

// forward declare
class QtEPComponent;

namespace internal {

// internal helper that creates a QtEPComponent (or derived) shim object containing a weak pointer
template <typename T>
struct BuildShimHelper
{
  epforceinline static QtEPComponent *Create(ep::Component *pComponent)
  {
    return new T(pComponent);
  }
};

} // namespace internal


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

} // namespace qt

Q_DECLARE_METATYPE(qt::QtEPComponent);

#endif // EP_COMPONENT_QT_H
