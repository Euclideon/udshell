#pragma once
#ifndef EP_COMPONENT_QT_H
#define EP_COMPONENT_QT_H

#include <QVariant>
#include <QJSValue>

#include "components/component.h"

namespace qt
{

class QtEPComponent : public QObject
{
  Q_OBJECT

public:
  QtEPComponent() : QObject(nullptr) {}
  QtEPComponent(ep::ComponentRef spComponent) : QObject(nullptr), spComponent(spComponent) {}
  QtEPComponent(const QtEPComponent &val) : QObject(nullptr), spComponent(val.spComponent) {}

  ep::ComponentRef GetComponent() const { return spComponent; }

  // methods to inspect internals
  //..

public:
  Q_INVOKABLE QVariant Get(const QString &name) const;
  Q_INVOKABLE void Set(const QString &name, QVariant val);

  // Ugly but necessary
  Q_INVOKABLE QVariant Call(const QString &name) const;
  Q_INVOKABLE QVariant Call(const QString &name, QVariant arg0) const;
  Q_INVOKABLE QVariant Call(const QString &name, QVariant arg0, QVariant arg1) const;
  Q_INVOKABLE QVariant Call(const QString &name, QVariant arg0,
    QVariant arg1, QVariant arg2) const;
  Q_INVOKABLE QVariant Call(const QString &name, QVariant arg0,
    QVariant arg1, QVariant arg2, QVariant arg3) const;
  Q_INVOKABLE QVariant Call(const QString &name, QVariant arg0,
    QVariant arg1, QVariant arg2, QVariant arg3, QVariant arg4) const;
  Q_INVOKABLE QVariant Call(const QString &name, QVariant arg0,
    QVariant arg1, QVariant arg2, QVariant arg3, QVariant arg4, QVariant arg5) const;
  Q_INVOKABLE QVariant Call(const QString &name, QVariant arg0,
    QVariant arg1, QVariant arg2, QVariant arg3, QVariant arg4,
    QVariant arg5, QVariant arg6) const;
  Q_INVOKABLE QVariant Call(const QString &name, QVariant arg0,
    QVariant arg1, QVariant arg2, QVariant arg3, QVariant arg4,
    QVariant arg5, QVariant arg6, QVariant arg7) const;
  Q_INVOKABLE QVariant Call(const QString &name, QVariant arg0,
    QVariant arg1, QVariant arg2, QVariant arg3, QVariant arg4,
    QVariant arg5, QVariant arg6, QVariant arg7, QVariant arg8) const;
  Q_INVOKABLE QVariant Call(const QString &name, QVariant arg0,
    QVariant arg1, QVariant arg2, QVariant arg3, QVariant arg4,
    QVariant arg5, QVariant arg6, QVariant arg7, QVariant arg8, QVariant arg9) const;

  Q_INVOKABLE void Subscribe(QString eventName, QJSValue func) const;

private:
  ep::ComponentRef spComponent;
};

} // namespace qt

Q_DECLARE_METATYPE(qt::QtEPComponent);

#endif // EP_COMPONENT_QT_H
