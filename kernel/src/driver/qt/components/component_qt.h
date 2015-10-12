#pragma once
#ifndef EP_COMPONENT_QT_H
#define EP_COMPONENT_QT_H

#include <QVariant>
#include <QJSValue>

#include "components/component.h"

// forward declare
namespace qt {
  class QtKernel;

  namespace internal {
    epResult SetupFromQmlFile(epInitParams initParams, qt::QtKernel *pKernel, ep::Component *pComponent, QObject **ppInternal);
  }


class QtEPComponent : public QObject
{
  Q_OBJECT

public:
  QtEPComponent() : QObject(nullptr), wpComponent(nullptr) {}
  QtEPComponent(ep::ComponentRef spComponent) : QObject(nullptr), spComponent(spComponent) { wpComponent = spComponent.ptr(); }
  QtEPComponent(const QtEPComponent &val) : spComponent(ep::ComponentRef(val.wpComponent)), wpComponent(val.wpComponent) {}
  ~QtEPComponent() {}

  ep::ComponentRef GetComponent() const { return ep::ComponentRef(wpComponent); }

  // methods to inspect internals
  //..

public:
  void Done() { emit completed(); }

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

private:
  friend epResult internal::SetupFromQmlFile(epInitParams initParams, qt::QtKernel *pKernel, ep::Component *pComponent, QObject **ppInternal);
  QtEPComponent(epWeakPtr<ep::Component> wpComp) : QObject(nullptr), wpComponent(wpComp) {}

  ep::ComponentRef spComponent;
  epWeakPtr<ep::Component> wpComponent; // used to avoid circular references
};

} // namespace qt

Q_DECLARE_METATYPE(qt::QtEPComponent);

#endif // EP_COMPONENT_QT_H
