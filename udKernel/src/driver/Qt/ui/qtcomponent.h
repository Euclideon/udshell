#pragma once
#ifndef QTCOMPONENT_H
#define QTCOMPONENT_H

#include "components/component.h"

#include <QQuickItem>


namespace ud
{
  class Kernel;
}

namespace qt
{

class QtComponent
{
public:
  QtComponent(ud::Component *pComponent, udString qml);
  ~QtComponent();

  QObject *QtObject() { return pQtObject; }
  const QObject *QtObject() const { return pQtObject; }

  QQuickItem *QuickItem() { return static_cast<QQuickItem*>(pQtObject); }

protected:

  static ud::ComponentDesc *CreateComponentDesc(const ud::ComponentDesc *pType);
  void PopulateComponentDesc(QObject *pObject);

  ud::Component *pComponent;
  QObject *pQtObject;
};

} // namespace qt

#endif  // QTCOMPONENT_H
