#pragma once
#ifndef UD_COMPONENT_QT_H
#define UD_COMPONENT_QT_H

#include <qobject.h>
#include <qvariant.h>
#include <qjsvalue.h>

#include "components/component.h"

namespace qt
{

class QtUDComponent : public QObject
{
  Q_OBJECT

public:
  QtUDComponent() : QObject(nullptr) {}
  QtUDComponent(ud::ComponentRef spComponent) : QObject(nullptr), spComponent(spComponent) {}
  QtUDComponent(const QtUDComponent &val) : QObject(nullptr), spComponent(val.spComponent) {}

  ud::ComponentRef GetComponent() const { return spComponent; }

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
  ud::ComponentRef spComponent;
};

} // namespace qt

Q_DECLARE_METATYPE(qt::QtUDComponent);

#endif // UD_COMPONENT_QT_H
