#pragma once
#ifndef UD_COMPONENT_QT_H
#define UD_COMPONENT_QT_H

#include <qobject.h>
#include <qvariant.h>
#include <qjsvalue.h>

#include "components/component.h"

namespace qt
{

class QUDComponent : public QObject
{
  Q_OBJECT

public:
  QUDComponent() : QObject(nullptr) {}
  QUDComponent(ud::ComponentRef spComponent) : QObject(nullptr), spComponent(spComponent) {}
  QUDComponent(const QUDComponent &val) : QObject(nullptr), spComponent(val.spComponent) {}

  ud::ComponentRef GetComponent() const { return spComponent; }

  // methods to inspet internals
  //..

public:
  Q_INVOKABLE QVariant Get(QString name) const;
  Q_INVOKABLE void Set(QString name, QVariant val);
  Q_INVOKABLE QVariant Call(QString name, QVariantList args) const;
  Q_INVOKABLE void Subscribe(QString eventName, QJSValue func) const;

private:
  ud::ComponentRef spComponent;
};

} // namespace qt

Q_DECLARE_METATYPE(qt::QUDComponent);

#endif // UD_COMPONENT_QT_H
