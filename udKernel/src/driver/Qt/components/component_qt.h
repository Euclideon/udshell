#pragma once
#ifndef UD_COMPONENT_QT_H
#define UD_COMPONENT_QT_H

#include <qobject.h>
#include <qvariant.h>
#include <qjsvalue.h>

#include "components/component.h"

namespace qt
{

class QtComponent : public QObject
{
  Q_OBJECT

public:
  QtComponent() : QObject(nullptr) {}
  QtComponent(ud::ComponentRef spComponent) : QObject(nullptr), spComponent(spComponent) {}
  QtComponent(const QtComponent &val) : QObject(nullptr), spComponent(val.spComponent) {}

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

Q_DECLARE_METATYPE(qt::QtComponent);

#endif // UD_COMPONENT_QT_H
